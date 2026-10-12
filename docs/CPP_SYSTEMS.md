# C++ Systems

All gameplay systems live in `Appalcahian/Source/Appalcahian`. They are building blocks:
the C++ handles the logic, you set values and wire events in Blueprints.

## First build

1. Install **Visual Studio 2022** with these workloads:
   - *Game development with C++* (tick **Unreal Engine installer** and a **Windows 11 SDK** in the details panel)
   - *.NET desktop development*
2. Close Unreal. Right-click `Appalcahian/Appalcahian.uproject` > **Generate Visual Studio project files**.
3. Double-click `Appalcahian.uproject`. When it says modules are missing or out of date, click **Yes** to rebuild.
   First compile takes a few minutes.
4. If it fails, open `Appalcahian.sln`, set the config to **Development Editor / Win64**, build, and send me the
   first error from the Output window.

After changing C++: close the editor and rebuild from Visual Studio (or use Live Coding with Ctrl+Alt+F11 for
small edits inside a .cpp; header changes need a full restart).

## Systems

Everything below shows up in the **Add Component** list under *Horror*, or as a parent class
when you make a new Blueprint.

### UHorrorGameSubsystem — story state
Always exists, nothing to place. In any Blueprint: *Get HorrorGameSubsystem* >
`SetFlag` / `HasFlag` (free-form names like `Fuse_Ventilation`, `ServerRoomScareDone`),
`ChooseEnding`, and the `OnFlagChanged` / `OnEndingChosen` events.

### UHorrorFlashlightComponent + UFlashlightProfile — the two lights
1. Content Browser > right-click > *Miscellaneous > Data Asset* > `FlashlightProfile`. Make two:
   - `DA_Flashlight_GodBeam`: Cone 70, Hotspot 0.5, Temperature 6500, Flicker 0
   - `DA_Flashlight_DCell`: Cone 35, Hotspot 0.3, Temperature 2800, Intensity much lower,
     Volumetric Scattering 2-4 (so the fog catches it), Flicker 0.1-0.2
2. In `BP_HorrorCharacter`, add `HorrorFlashlight` as a child of the camera. Set Starting Profile to God Beam.
3. Flashlight key > `ToggleFlashlight`.
4. Act 4.1: `KillFlashlight`. Act 4.2 corpse pickup: `ReplaceFlashlight(DA_Flashlight_DCell)`.

Intensity is in candelas. Tune the numbers in-editor against your exposure; the defaults are a starting point.

### UHorrorControlLockComponent — the server room paralysis
Add to `BP_HorrorCharacter`. In the look input handler, multiply the look values by `GetLookScale`.
Call `LockMovement(1.5, 0.08)` at the reflection beat.

### IHorrorInteractable + UHorrorInteractionComponent — using things
1. Add `HorrorInteraction` to `BP_HorrorCharacter`.
2. Make an `IA_Interact` input action, add it to the mapping context.
   *Started* > `BeginInteract`, *Completed* and *Canceled* > `EndInteract`.
3. On any usable Blueprint: *Class Settings > Implemented Interfaces > HorrorInteractable*, then override:
   - `Interact` — what happens
   - `GetInteractPrompt` — "Pull fuse", "Read"
   - `GetHoldDuration` — 0 for a press, e.g. 3.0 for a valve
   - `OnHoldProgress` — rotate the valve wheel from Alpha
   - `CanInteract` — e.g. false once the fuse is already pulled
4. Bind `OnFocusChanged` on the component to show/hide your prompt widget.

### AHorrorWatcher — the creature as presence
Make a Blueprint child (`BP_Watcher`), add the silhouette mesh with **no collision**, origin at its feet.
Drag the Sight Points widgets to chest/head height if your mesh differs.

| Behavior | Use for |
|---|---|
| VanishWhenUnseen | Every glimpse up to 5.1. Look away or something blocks it: gone. The Act 3 pillar beat works out of the box. |
| StayWhenUnseen | 5.3, the window. Bind `OnLookedBackAt`, hold 1-2s, then do what you want. |
| Accompany | 5.4 onward. Repositions behind the player whenever unseen, never pops into view. |

Turn `Start Active` off and call `ActivateWatcher` from a trigger to place one that waits.
"Seen" means on-screen and not blocked. It does not check lighting.

### UReflectionScareComponent — eyes in the glass
1. In each reflective surface material, add a scalar parameter `EyesOpacity` (default 0) that blends an
   eyes texture on top of the reflection.
2. Add the component to the surface actor. It arms once the `ServerRoomScareDone` flag is set (set that at the
   end of the server room scene).
3. For the valve station 3 beat, call `ForceScare` directly.

Defaults: 5% per look, 90s shared cooldown between any two surfaces, 1 frame.

### UHorrorFootstepComponent — steps and the extra step
Add to `BP_HorrorCharacter`. Set Default Step Sound and fill Surface Step Sounds once you've set up physical
surfaces (Project Settings > Physics). At 5.4 call `SetEchoEnabled(true)`. `GetStopEchoCount` tells you when
the player has tested it twice.

Use a single Sound Wave for Echo Sound, not a randomizing Sound Cue. The beat only works if the extra step
sounds exactly the same every time.

## Design decisions on record
- **Fuel rods are spent**, not missing. The restart error is `CORE REACTIVITY INSUFFICIENT` / fuel depleted,
  not "not detected".
- **True ending:** the player seals themselves in with it. Stage the seal so a door closes visibly behind them
  before they look up at the glass.
- **The command note:** forestry agency, sent to tell the kids to get off the radio.
