# Gameplay Action Framework

A lightweight, modular gameplay ability system for Unreal Engine 5. Inspired by GAS but designed for single‑player projects — no replication, minimal overhead.

## Features

- **Gameplay Actions** — Blueprintable actions with tags, cost, cooldown, cancel policy, and input binding
- **Gameplay Effects** — Instant/Duration/Periodic/Infinite effects with additive, multiplicative, and override modifiers
- **Stacking** — Aggregate by source or target with configurable stack limits
- **Attribute Sets** — Custom attribute containers with `PreAttributeChange` / `PostAttributeChange` callbacks
- **Magnitude Calculation** — Blueprint-native custom magnitude logic per modifier
- **Tags** — Full `FGameplayTag` support with application requirements, blocked tags, and granted tags
- **Blueprint Library** — Helper functions for applying effects, managing tags, and reading attributes from any actor

## Quick Start

1. Enable the plugin in your project.
2. Create a **Gameplay Attribute Set** blueprint with your attributes.
3. Add a **Gameplay Action Component** to your character.
4. Add **Default Attributes** in the component settings.
5. Create a **Gameplay Action** blueprint and implement `OnExecute` / `OnEnd`.
6. Create a **Gameplay Effect** blueprint to define modifiers.
7. Call `Add Gameplay Action` and `Try Activate Action by Tag` / `Press Input ID` from your character blueprint.

## Requirements

- Unreal Engine 5.3+
- `GameplayTags` and `GameplayTasks` engine modules

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

### Gameplay Effects

`UGameplayEffect` is a `UDataAsset` defining attribute modifications. Effects are applied via `ApplyGameplayEffectSpecToSelf` / `ApplyGameplayEffectToSelf`.

**Duration** — controlled by `DurationPolicy`:
- `Instant` — modifiers applied immediately, no timer, no revert
- `HasDuration` — modifiers applied, `DurationMagnitude` seconds timer, `RevertEffectModifiers` on expiry
- `Infinite` — modifiers applied, never expires (removed manually)

**Period** — if `Period > 0.0f`, modifiers are applied via `SetNumericValue` (CurrentValue only) on every tick — no BaseValue tracking, no revert on expiry.

**Modifier operations:**
- `Additive` — `CurrentValue += Magnitude`
- `Multiplicative` — `CurrentValue *= Magnitude`
- `Override` — `CurrentValue = Magnitude`

**Magnitude priority** (per `FGameplayModifierInfo`):
1. `MagnitudeCalculation` — if set, `CalculateMagnitude(EffectSpec, ActionComponent)` is called
2. `DataTag` — if a `SetByCaller` magnitude with matching tag exists in the spec
3. `Magnitude` — raw float value

**Stacking** — when `StackingPolicy != None`, applying an effect of the same class refreshes the existing effect instead of creating a new one. `StackLimitCount` caps the number of stacks. Supported policies:
- `AggregateBySource` — stack per source
- `AggregateByTarget` — stack per target

**Tags** — effects can grant tags on application, require tags on the target (`ApplicationRequiredTags`), or be blocked by tags on the target (`ApplicationBlockedTags`).

### Cost and Cooldown

Actions can optionally define:

- **Cost effect** (`CostEffect`) — applied before activation. `CanApplyCost` checks if the cost would bring any attribute below 0. Override `OnExecuteAction` and call `CommitCost()` manually to enforce it.
- **Cooldown effect** (`CooldownEffect`) — applied when the action ends. `RequestExecuteAction` checks `HasActiveEffect(CooldownEffect)` and blocks activation if the cooldown is still active.

Both are regular `UGameplayEffect` assets — use Instant duration with Additive modifiers on the relevant attributes.

### Attribute Sets

`UGameplayAttributeSet` is a container for `FGameplayAttributeData` properties. Define your attributes as `UPROPERTY` members of type `FGameplayAttributeData`:

```cpp
UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Attributes")
FGameplayAttributeData Health;
UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Attributes")
FGameplayAttributeData Mana;
```

BaseValue is the persistent value; CurrentValue is the runtime value (modified by effects). `SetBaseValue` automatically syncs CurrentValue. Effects modify CurrentValue; on expiry, the modifier is reverted from CurrentValue and BaseValue is unaffected.

Callbacks:
- `PreAttributeChange(Attribute, NewValue)` — called before a set; modify `NewValue` for clamping
- `PostAttributeChange(Attribute, NewValue, OldValue)` — called after a set

### Magnitude Calculation

Create a child Blueprint of `UGameplayEffectMagnitudeCalculation` and override `CalculateMagnitude`. Assign it to a `MagnitudeCalculation` field in `FGameplayModifierInfo`. The calculation is evaluated at effect application time with access to the full `FGameplayEffectSpec` and the target's `UGameplayActionComponent`.

### Blueprint Library

`UGameplayActionBlueprintLibrary` provides static functions callable from any Blueprint:

| Function | Purpose |
|----------|---------|
| `GetGameplayActionComponent` | Gets the component from an actor |
| `GetOwnedGameplayTagsFromActor` | Returns tags on an actor |
| `AddOwnedGameplayTagsToActor` | Adds tags to an actor |
| `RemoveOwnedGameplayTagsFromActor` | Removes tags from an actor |
| `GetAttributeValueFromActor` | Gets an attribute's current value |
| `MakeGameplayEffectSpec` | Creates an effect spec from a class |
| `AddSetByCallerMagnitude` | Adds a set-by-caller magnitude to a spec |
| `ApplyGameplayEffectSpecToActor` | Applies an effect spec to an actor |
| `ApplyGameplayEffectToActor` | Applies an effect class to an actor |

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

## Structure

| Class | Role |
|-------|------|
| `UGameplayActionComponent` | Owned by the actor; manages actions, effects, tags, and attributes |
| `UGameplayAction` | Base for all actions — Default (reusable) or Triggered (one‑shot) |
| `UGameplayEffect` | Data asset defining modifiers, duration, period, stacking, and tags |
| `UGameplayAttributeSet` | Container for `FGameplayAttributeData` properties |
| `UGameplayEffectMagnitudeCalculation` | Custom magnitude calculation per modifier |

## Notes

- All effect modifiers on periodic effects (`Period > 0`) operate on CurrentValue only — no BaseValue tracking, no revert on expiry.
- To inspect running effects, use `HasActiveEffect(Class)` or `HasActiveEffectSpecHandle(Handle)`.
- Cost and cooldown effects are optional — if not set, no cost/cooldown is enforced.
