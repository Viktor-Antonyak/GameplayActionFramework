# Gameplay Action Framework

A lightweight, modular gameplay ability system for Unreal Engine 5. Inspired by GAS but designed for single‑player projects — no replication, minimal overhead.

## Features

- **Gameplay Actions** — Blueprintable actions with tags, cost, cooldown, cancel policy, and input binding
- **Gameplay Effects** — Instant/Duration/Periodic/Infinite effects with additive, multiplicative, and override modifiers
- **Stacking** — Aggregate by source or target with configurable stack limits
- **Attribute Sets** — Custom attribute containers with `PreAttributeChange` / `PostAttributeChange` callbacks
- **Magnitude Calculation** — Blueprint-native custom magnitude logic per modifier
- **Execution Calculation** — Full control over effect application, replaces standard modifier loop
- **Tags** — Stackable `FGameplayTag` system with reference counting (quantity), application requirements, blocked tags, and granted tags
- **Init Actions** — Actions auto-initialized on BeginPlay from the component's array, no Blueprint graph needed
- **Blueprint Library** — Helper functions for applying effects, managing tags, reading attributes, and sending gameplay events from any actor
- **Gameplay Events** — `SendGameplayEvent` + `WaitGameplayEvent` task for tag-based event broadcasting across the component
- **Debug Visualization** — Gameplay Debugger `GAF` category (plus the deprecated `DebugGAF` on-screen overlay) to inspect actions, effects, tags, and attributes

## Quick Start

1. Enable the plugin in your project.
2. Create a **Gameplay Attribute Set** blueprint with your attributes.
3. Add a **Gameplay Action Component** to your character.
4. Implement **Gameplay Action Interface**
5. Add **Default Attributes** in the component settings.
6. Create a **Gameplay Action** blueprint and implement `OnExecute` / `OnEnd`.
7. Create a **Gameplay Effect** blueprint to define modifiers.
8. Call `Add Gameplay Action` and `Try Activate Action by Tag` / `Press Input ID` from your character blueprint.

## Requirements

- Unreal Engine 5.3+
- `GameplayTags` and `GameplayTasks` engine modules
- `StructUtils` plugin (for `FInstancedStruct`)

> **Works alongside Epic's built-in GameplayAbilities plugin (fixed in v1.5).** Versions ≤1.4 were incompatible with GAS: several reflected type names (`EGameplayModOp`, `FGameplayEffectSpec`, `FActiveGameplayEffectHandle`, `FGameplayAttribute`, etc.) collided at the UHT level and caused compile errors. From **v1.5** all reflected types were renamed under the `GameplayAction*` prefix, so the two can coexist.
>
> **Renamed types:** `UGameplayEffect` → `UGameplayActionEffect`, `FGameplayEffectSpec` → `FGameplayActionEffectSpec`, `FActiveGameplayEffect(Handle)` → `FActiveGameplayActionEffect(Handle)`, `EGameplayModOp` → `EGameplayActionEffectModOp`, `FGameplayModifierInfo` → `FGameplayActionEffectModifierInfo`, `UGameplayAttributeSet` → `UGameplayActionAttributeSet`, `FGameplayAttribute(Data)` → `FGameplayActionAttribute(Data)`, `UGameplayEffectMagnitudeCalculation` → `UGameplayActionEffectMagnitudeCalc`, `UGameplayEffectExecutionCalculation` → `UGameplayActionEffectExecutionCalc`, and their Blueprint / editor counterparts. Core redirects in `Config/DefaultGameplayActionFramework.ini` migrate existing assets automatically.

## How It Works

### Action Lifecycle

1. **Registration** — `AddGameplayAction(TSubclassOf<UGameplayAction>, Level)` creates an instance of the action and stores it as an `FGameplayActionSpec` in the component's `AddedActions` array. `InitializeAction` is called. The `ActionType` determines behaviour:
   - `Default` — stays registered, can be executed multiple times
   - `Triggered` — cannot be added; use `TriggerActionByClass` instead

2. **Activation** — `TryActivateActionBySpec` / `TryActivateActionsByTag` / `PressInputID` call `RequestExecuteAction`:
   - Checks `CanExecuteAction` (overridable in Blueprint)
   - Checks required/blocked tags on the owner
   - Checks if a cooldown effect is still active
   - Resolves cancel policy: `Block` (fail if any blocking action can't cancel) or `IgnoreFailed` (cancel what it can, activate anyway)
   - If all checks pass: `bIsActive = true`, added to `ActiveActions`, grant tags applied, `OnExecuteAction` called

3. **Triggered actions** — `TriggerActionByClass` creates a new instance, calls `RequestTriggerAction` with a payload (`FInstancedStruct`), executes once, and is destroyed on end.

4. **Ending** — `RequestEndAction` calls `EndAction`: removes from active list, removes grant tags, applies cooldown effect if set, calls `OnEndAction(bool bWasCanceled)`.

5. **Input release** — `ReleaseInputID(InputID)` calls `RequestReleaseInputID`, which fires `OnInputIDReleased` if the action is currently active. Unlike earlier versions it does **not** end the action — handle press/release semantics (e.g. hold-to-channel) in `OnExecuteAction` / `OnInputIDReleased`.

### Init Actions

`FActionInitializationData` pairs a `TSubclassOf<UGameplayAction>` with a `Level`. Fill `InitialActions` in the component's details panel to auto-initialize actions on `BeginPlay`.

### Gameplay Effects

`UGameplayActionEffect` is a `UDataAsset` defining attribute modifications. Effects are applied via `ApplyGameplayEffectSpecToSelf` / `ApplyGameplayEffectToSelf`. Each `FGameplayActionEffectSpec` carries a `Level` (default 1) for level-scaled effects and an optional `SourceActor` — the instigator (used for source-based stacking and available to Magnitude/Execution calculations). Pass it when applying via `MakeGameplayEffectSpec` / `ApplyGameplayEffectToActor` / `ApplyGameplayEffectToSelf`.

**Duration** — controlled by `DurationPolicy`:
- `Instant` — modifiers are evaluated once and applied **directly to the attribute's BaseValue** (additive/multiplicative/override on the base), no timer, no revert. Good for damage, healing, cost/cooldown.
- `HasDuration` — modifiers are registered with the attribute **aggregator** (see below) for the effect's lifetime, then removed on expiry.
- `Infinite` — same as `HasDuration` but never expires (removed manually).

**Duration/Infinite modifiers and the aggregator** — instead of mutating a value in place, each active modifier is registered with a per-attribute `FGameplayActionAttributeAggregator`. The attribute's `CurrentValue` is recomputed as:

```
CurrentValue = (BaseValue + Sum(Additive)) * Product(Multiplicative)
```

or, if any `Override` modifier is active, `CurrentValue = <last Override>`. This means overlapping Duration/Infinite effects on the same attribute compose correctly and can be added/removed independently regardless of application order — there is no order-dependent in-place mutation and no "revert to saved base" step.

**Period** — if `Period > 0.0f`, the modifiers are re-applied on every tick via `SetBaseValue` (applied against the current base each tick). Tick skipped while the effect is inhibited.

**Modifier operations:**
- `Additive` — Instant/Periodic: `BaseValue += Magnitude`; Duration/Infinite: added to the additive sum
- `Multiplicative` — Instant/Periodic: `BaseValue *= Magnitude`; Duration/Infinite: multiplied into the product
- `Override` — Instant/Periodic: `BaseValue = Magnitude`; Duration/Infinite: takes over the final value entirely

**Execution Calculation** — runs after all modifiers. Useful for reading final attribute values and applying follow-up logic (e.g., read health after damage → apply bleed).

**Magnitude priority** (per `FGameplayActionEffectModifierInfo`, only when no Execution is set):
1. `MagnitudeCalculation` — if set, `CalculateMagnitude(EffectSpec, ActionComponent)` is called
2. `DataTag` — if a `SetByCaller` magnitude with matching tag exists in the spec
3. `Magnitude` — raw float value

**Stacking** — when `StackingPolicy != None`, applying an effect of the same class reuses the existing active effect and increments its `StackCount`, re-applying the effect's modifiers (each stack becomes its own aggregator contribution) until `StackLimitCount` is reached. Supported policies:
- `AggregateBySource` — stack per `SourceActor` (two separate casters stack independently)
- `AggregateByTarget` — stack per target (all sources share one stack)

**Inhibition** — `SetEffectInhibited(Handle, bInhibited)` suppresses an active Duration/Infinite effect's contribution to attributes **without removing it**: its handle, duration timer, and stack count stay intact, and its modifiers are removed from (or re-added to) the attribute aggregator. Query state with `IsEffectInhibited(Handle)`. Notes:
- Applies to the **entire** effect, all stacks at once — there is no per-stack inhibition.
- Instant effects are rejected (they have no ongoing modifiers to toggle).
- Periodic effects skip their tick while inhibited.
- The framework only toggles on your signal — it's up to you to decide when (e.g. tag-driven conditions).

**Tags** — effects can grant tags on application, require tags on the target (`ApplicationRequiredTags`), or be blocked by tags on the target (`ApplicationBlockedTags`). Tags now use a stackable container with reference counting — `AddOwnedGameplayTags` / `RemoveOwnedGameplayTags` accept a `Quantity` parameter, and `GetOwnedGameplayTagCount(Tag)` returns the current stack count.

**Component delegates** — the component exposes Blueprint-assignable events: `OnActionActivated`, `OnActionEnded`, `OnEffectAdded`, `OnEffectRemoved`, `OnAttributeChanged`, `OnGameplayTagsAdded`, `OnGameplayTagsRemoved`, `OnGameplayActionEvent`.

**Gameplay events** — `SendGameplayEvent(EventTag)` broadcasts `OnGameplayActionEvent` on the component; from Blueprint use `SendGameplayEventToActor(Actor, EventTag)`. Consume events with the `WaitGameplayEvent` task (see Tasks below).

**Effect time queries** — three `BlueprintPure` helpers on active effects: `GetEffectElapsedTime(Handle)` (seconds since application), `GetEffectRemainingTime(Handle)` (seconds until expiry; `-1` for infinite/invalid), `GetEffectDuration(Handle)` (total duration; `-1` for infinite, `0` for invalid).

**Native (C++) delegates on tasks** — every task exposes an additional native multicast delegate next to its Blueprint one (e.g. `OnMovementFinishedNative`, `OnAttributeChangedNative`, `OnTagAddedNative`, `OnCompletedNative`, `OnGameplayEventNative`) for C++ binding without Blueprint.

### Cost and Cooldown

Actions can optionally define:

- **Cost effect** (`CostEffect`) — applied before activation. `CanApplyCost` checks if the cost would bring any attribute below 0. Override `OnExecuteAction` and call `CommitCost(CostLevel = 1)` manually to enforce it.
- **Cooldown effect** (`CooldownEffect`) — applied when the action ends. `RequestExecuteAction` checks `HasActiveEffect(CooldownEffect)` and blocks activation if the cooldown is still active. `CommitCooldown(CooldownLevel = 1)` applies it at the given level.

Both are regular `UGameplayActionEffect` assets — use Instant duration with Additive modifiers on the relevant attributes.

### Attribute Sets

`UGameplayActionAttributeSet` is a container for `FGameplayActionAttributeData` properties. Define your attributes as `UPROPERTY` members of type `FGameplayActionAttributeData`:

```cpp
UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Attributes")
FGameplayActionAttributeData Health;
UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Attributes")
FGameplayActionAttributeData Mana;
```

Use the `GAMEPLAY_ACTION_ATTRIBUTE_ACCESSORS(ClassName, PropertyName)` macro (below the attribute declarations) to generate GAS-style accessors. A plain `GAMEPLAY_ACTION_ATTRIBUTE_ACCESSOR` generates only the static attribute lookup:

```cpp
GAMEPLAY_ACTION_ATTRIBUTE_ACCESSORS(UMyAttributeSet, Health)
GAMEPLAY_ACTION_ATTRIBUTE_ACCESSORS(UMyAttributeSet, Mana)
```

For each attribute this generates:

```cpp
// from GAMEPLAY_ACTION_ATTRIBUTE_ACCESSOR:
static FGameplayActionAttribute GetHealthAttribute();  // cached FindFProperty lookup

// additionally from GAMEPLAY_ACTION_ATTRIBUTE_ACCESSORS:
float GetHealth() const;        // CurrentValue
float GetBaseHealth() const;    // BaseValue
void  SetHealth(float NewVal);  // SetNumericValue — Pre/Post callbacks + broadcast
void  InitHealth(float NewVal); // InitNumericValue — no callbacks (setup only)
```

BaseValue is the persistent source of truth; CurrentValue is the runtime value. Duration/Infinite effects never touch BaseValue — they contribute through the aggregator and CurrentValue is recomputed from `BaseValue + modifiers`. Instant and Periodic effects mutate BaseValue directly (via `SetBaseValue`), and CurrentValue is recalculated after each change.

> **Note on `FGameplayActionAttribute` parameters:** when you expose `FGameplayActionAttribute` in an async/`BlueprintInternalUseOnly`-style (Task, Latent) node signature, use a plain `FGameplayActionAttribute` parameter — **not** `const FGameplayActionAttribute&`. A `const` reference breaks the Blueprint attribute picker, so the selected value will not be correctly written. Use `FGameplayActionAttribute` (by value) so the PickAttribute dropdown works.

Callbacks:
- `PreAttributeChange(Attribute, NewValue)` → returns `float` — called before a current-value change; return the clamped value
- `PostAttributeChange(Attribute, NewValue, OldValue)` — called after a current-value change
- `PreAttributeBaseChange(Attribute, NewValue)` → returns `float` — called before a **BaseValue** change (Instant/Periodic effects); return the clamped value

The component broadcasts `OnAttributeChanged(Attribute, NewValue, OldValue)` whenever a value actually changes — whether from an aggregator recompute (Duration/Infinite effects, `SetAttributeValue`) or a `SetBaseValue` (Instant/Periodic effects). It only fires on real changes (near-equal values are skipped), so Blueprints can react to attribute updates without polling.

### Magnitude Calculation

Create a child Blueprint of `UGameplayActionEffectMagnitudeCalc` and override `CalculateMagnitude`. Assign it to a `MagnitudeCalculation` field in `FGameplayActionEffectModifierInfo`. The calculation is evaluated at effect application time with access to the full `FGameplayActionEffectSpec` and the target's `UGameplayActionComponent`.

### Execution Calculation

Create a child Blueprint of `UGameplayActionEffectExecutionCalc` and override `Execute`. Assign it to the `ExecutionCalculation` field on the effect itself. It runs **after** all modifiers, so you can read final attribute values to apply follow-up logic.

Use Execution Calculation when:
- Damage reduces to armour/damage mitigation formulas
- Life steal or damage → healing chains
- Conditional logic: "if poisoned → double damage"
- Any scenario where one modifier's result affects another

```cpp
// Example: damage with armour mitigation + bleed
void UMyExecution::Execute_Implementation(
    const FGameplayActionEffectSpec& Spec,
    UGameplayActionComponent* Target)
{
    float RawDamage = Spec.GetSetByCallerMagnitude(DamageTag);
    float Armor = Target->GetAttributeValue(ArmorAttr);
    float FinalDamage = RawDamage * (100.0f / (100.0f + Armor));

    Target->SetAttributeValue(HealthAttr,
        Target->GetAttributeValue(HealthAttr) - FinalDamage);

    float Bleed = FinalDamage * FMath::FRandRange(0.05f, 0.10f);
    FGameplayActionEffectSpec BleedSpec;
    BleedSpec.Effect = BleedEffectCDO;
    BleedSpec.SetSetByCallerMagnitude(BleedTag, Bleed);
    Target->ApplyGameplayEffectSpecToSelf(BleedSpec);
}
```

### Blueprint Library

`UGameplayActionBlueprintLibrary` provides static functions callable from any Blueprint:

| Function | Purpose |
|----------|---------|
| `GetGameplayActionComponent` | Gets the component from an actor |
| `GetOwnedGameplayTagsFromActor` | Returns tags on an actor |
| `AddOwnedGameplayTagsToActor(Tags, Quantity=1)` | Adds tags to an actor with optional stack count |
| `RemoveOwnedGameplayTagsFromActor(Tags, Quantity=1)` | Removes tags from an actor with optional stack count |
| `SendGameplayEventToActor(EventTag)` | Sends a gameplay event to an actor's component (broadcasts `OnGameplayActionEvent`) |
| `GetAttributeValueFromActor` | Gets an attribute's current value |
| `MakeGameplayEffectSpec(Effect, Level=1, SourceActor=null)` | Creates an effect spec from a class with optional level and source |
| `AddSetByCallerMagnitude` | Adds a set-by-caller magnitude to a spec |
| `ApplyGameplayEffectSpecToActor` | Applies an effect spec to an actor |
| `ApplyGameplayEffectToActor(Effect, Level=1, SourceActor=null)` | Applies an effect class to an actor with optional level and source |
| `FindFragment(FragmentsHandle, Fragment, IsFound)` | Returns a fragment of the given class from a fragments handle |

### Gameplay Action Tasks

Tasks are lightweight `UGameplayTask`-based helpers that run on the owning `UGameplayActionComponent`. They let Blueprints wait for gameplay events or drive movement/montages without manual bookkeeping. Each task is created by a static factory that takes a `WorldContextObject` (either a `UGameplayAction` or any actor that implements `IGameplayActionInterface`); bind the output delegates and the task handles its own lifecycle and cleanup (unbind, root-motion removal, montage stop).

| Task | Purpose |
|------|---------|
| `WaitAttributeChanged(Attribute, bOnlyTriggerOnce)` | Fires `OnAttributeChanged` whenever the watched attribute's value changes on the component |
| `WaitGameplayTagAdded(Tag, bExactTag, bOnlyTriggerOnce)` | Fires `OnTagAdded` when a matching tag is added |
| `WaitGameplayTagRemoved(Tag, bExactTag, bOnlyTriggerOnce)` | Fires `OnTagRemoved` when a matching tag is removed |
| `WaitGameplayEvent(Tag, bExactTag, bOnlyTriggerOnce)` | Fires `OnGameplayEvent` when `SendGameplayEvent(EventTag)` matches the tag |
| `PlayMontageAndWaitNotify(InMesh, Montage, PlayRate, StartSection, StartTimeSeconds, bStopMontageAtEnd)` | Plays a montage and fires `OnCompleted` / `OnBlendedOut` / `OnInterrupted` / `OnCancelled`, plus `OnNotifyBegin` / `OnNotifyEnd` for Branching Point notifies |
| `MoveConstant(Direction, Speed, Duration, SpeedOverTime)` | Moves an Actor at a constant velocity for a fixed duration |
| `MoveToLocation(TargetLocation, Duration, EasingCurve)` | Moves an Actor to a world location over time |
| `RootMoveConstant(RootTaskName, Direction, Speed, Duration, ...)` | Applies constant-velocity root motion to a Character |
| `RootMoveToLocation(RootTaskName, TargetLocation, Duration, PathOffsetCurve, ...)` | Moves a Character to a world location via root motion |

Notes:

- Movement tasks come in Actor and Root (Character) flavours — use Actor variants on plain `AActor` and Root variants on `ACharacter`, not the other way around (see class docs).
- Movement tasks broadcast `OnMovementFinished` on success and `OnMovementInterrupted` if destroyed before completing.
- Tag tasks fire once per matching tag inside a single added/removed container broadcast; `bOnlyTriggerOnce` ends the task after the first broadcast that contains a match (even if several tags matched).
- Root tasks accept root-motion settings (`RootMotionMode`, `RootMotionFinishVelocityMode`, gravity) and remove the root-motion source on destroy.
- Tasks don't tick (`bTickingTask = false`) and are created with priority 127; each task also broadcasts a native (`*Native`) delegate alongside the Blueprint one for C++ binding.

### Cancel Policy

`EGameplayActionCancelPolicy` controls behaviour when activation would cancel other running actions:
- `Block` — activation fails if any action with a matching `CancelOtherActions` tag cannot be cancelled
- `IgnoreFailed` — cancels what it can and activates regardless

Tags involved:
- `GameplayActionTag` — this action's own identifier
- `BlockOtherActions` — prevents matching actions from activating
- `CancelOtherActions` — running actions matching these tags will be cancelled on activation
- `RequireTags` — target must have all these tags
- `BlockedByTags` — target must not have any of these tags
- `GrantTags` — applied to the target while the action is active

### Action Lifecycle Events

`UGameplayAction` exposes two `BlueprintImplementableEvent` hooks:

- `OnActionInitialize` — fires after the action's spec is created (`InitializeAction`), once per instance
- `OnActionDeinitialize` — fires on `DeinitializeAction` (e.g. component `EndPlay`), before the action is released

These complement the native `InitializeAction` / `DeinitializeAction` overrides for Blueprint wiring.

### Gameplay Action Fragments

`UGameplayActionFragment` is an abstract `UObject` base class (child Blueprints can be edited in-place via `EditInlineNew` / `DefaultToInstanced`) for the data-oriented approach — child Blueprints hold optional custom data that can be attached to a `FGameplayActionFragmentsHandle`. Use `FindFragment(Handle, FragmentClass, IsFound)` (Blueprint library, with `ExpandBoolAsExecs`) to look up a fragment and get its data:

```cpp
FGameplayActionFragmentsHandle Handle;
// ... fill Handle.FragmentSlots with fragment instances ...

UGameplayActionFragment* Fragment = UGameplayActionBlueprintLibrary::FindFragment(Handle, UMyFragment::StaticClass(), bIsFound);
if (bIsFound) { /* cast to UMyFragment and use its data */ }
```

This keeps gameplay data separate from behaviour and lets a single action read different configuration depending on which fragments were attached.

## Structure

| Class | Role |
|-------|------|
| `UGameplayActionComponent` | Owned by the actor; manages actions, effects, tags (via `FGameplayTagStackableContainer`), and attributes |
| `UGameplayAction` | Base for all actions — Default (reusable) or Triggered (one‑shot) |
| `UGameplayActionEffect` | Data asset defining modifiers, duration, period, stacking, and tags |
| `FActiveGameplayActionEffectsContainer` | Map of active effects by handle; owns stacking and inhibition lookups |
| `FGameplayActionAttributeAggregator` | Per-attribute recomputation from BaseValue + active Duration/Infinite modifiers |
| `UGameplayActionAttributeSet` | Container for `FGameplayActionAttributeData` properties |
| `UGameplayActionEffectMagnitudeCalc` | Custom magnitude calculation per modifier |
| `UGameplayActionEffectExecutionCalc` | Full control over effect application (replaces modifiers) |
| `UGameplayActionFragment` | Object base for the data-oriented approach — optional custom data queried via `FindFragment` |
| `IGameplayActionInterface` | Interface for actors; exposes `GetGameplayActionComponent()` |
| `UGameplayActionTask` | Base for Blueprint tasks running on the component — waiting for events, movement, and montage playback |
| `FGAFGameplayDebuggerCategory` | Gameplay Debugger category (`GAF`) rendering live action/effect/tag/attribute data; non-Shipping builds only |

## Notes

- Periodic effects (`Period > 0`) re-apply their modifiers to BaseValue on every tick; tick is skipped while the effect is inhibited.
- Tags are reference-counted via `FGameplayTagStackableContainer`. Add/Remove accept a `Quantity` parameter; use `GetOwnedGameplayTagCount(Tag)` to query stack depth.
- To inspect running effects, use `HasActiveEffect(Class)` or `HasActiveEffectSpecHandle(Handle)`.
- Cost and cooldown effects are optional — if not set, no cost/cooldown is enforced.
- If you see the warning "*plugin was designed for build 5.3.0*", either ignore it (safe) or remove `EngineVersion` from `.uplugin` to clear it.
- `InitializeAction` / `DeinitializeAction` are `virtual` — override them in subclasses to hook into the lifecycle.

## Debug Visualization

Since **1.6** debug info is exposed through the engine's built-in **Gameplay Debugger**: press `'` (apostrophe) in-game and pick the **GAF** category. It shows data for the actor currently replicated/selected in the debugger (line-traced under your crosshair by default), laid out in a grid:

- **Grant Actions** — list of registered actions; green = currently active, white = inactive
- **Triggered Actions** — running one‑shot actions
- **Active Effects** — effect name, stack count, remaining time (or "Infinite"); inhibited effects marked `[INHIBITED]`
- **Owned Tags** — all gameplay tags on the actor
- **Attributes** — per attribute set: each attribute with `Base=` and `Current=`

Implementation: `FGAFGameplayDebuggerCategory` (in `GameplayActionFrameworkDebugger.h`) registers itself as a `"GAF"` category via `IGameplayDebugger::RegisterCategory` on module startup and is compiled only for non-Shipping builds (`WITH_GAMEPLAY_DEBUGGER`). The component exposes read-only accessors (`GetAddedActions`, `GetActiveEffects`, `GetAttributeSets`, ...) used by the category.

### Legacy overlay (deprecated)

The old on-screen overlay is still available but deprecated: type `DebugGAF` to toggle it. It relies on `UGameplayActionFrameworkDebugSubsystem` + `UDebugDrawService` and `UGameplayActionComponent::DisplayDebug` — both marked `UE_DEPRECATED` since 1.6 and will be removed in a future release.
