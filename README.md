# Click Indicators (Geode mod)

Two bars that light up the instant you press and release. Player 1 is
green by default; Player 2 stays dark until it actually sees a
dual-mode input, then lights up red. Both colors, bar size, and the
click/release sounds are configurable from the mod's settings page
in-game.

This reflects your *own* input as you make it — it doesn't read level
geometry or tell you what's coming, so it works the same on every
level automatically.

## Getting a `.geode` file without installing anything (includes Android)

This is the easiest path, and the one I'd actually recommend, since it
needs no local toolchain — not even for Android, which normally
requires the NDK:

1. Create a new repo on GitHub (can be private).
2. Push everything in this folder to it, including the hidden
   `.github/` folder — that's what has the build workflow.
3. Go to the repo's **Actions** tab. A workflow called "Build Geode
   Mod" should already be running (it triggers on every push).
4. Once it finishes (a few minutes), open that workflow run and
   scroll to **Artifacts** at the bottom — there'll be one named
   after the mod, containing the finished `.geode` file. It's built
   automatically for Windows, macOS, iOS, and both Android
   architectures (32 and 64-bit) combined into one file that works on
   whichever platform GD is running on.
5. Download the artifact, unzip it, and you've got your `.geode` file
   — copy it into GD's mods folder (or double-tap it in the Geode app
   on Android) to install it.

The one thing you still need to do yourself: add `click.ogg` and
`release.ogg` to `resources/` *before* pushing (see below) — the
workflow will build fine without them, but sound will silently do
nothing at runtime if they're missing.

## Before you build locally

1. Install the [Geode CLI](https://docs.geode-sdk.org/getting-started/)
   and the Geode SDK, and make sure the `GEODE_SDK` environment
   variable points at your SDK install (the CLI setup does this for
   you).
2. Add two short sound files to `resources/`:
   - `resources/click.ogg` — plays on press
   - `resources/release.ogg` — plays on release
   Anything short (a few hundred ms) and punchy works well. I didn't
   generate these since I can't produce audio files — grab two
   royalty-free UI click sounds (freesound.org has plenty under CC0)
   and drop them in, or record your own.

## Build

From the project folder:

```
geode build
```

This produces a `.geode` file (check the `build/` output directory).
Drop that file into your GD mods folder, or just double-click it if
you have the Geode Installer associated with `.geode` files.

## Things worth double-checking

I wrote and reasoned through this against the current Geode docs, but
I don't have a way to actually compile it against the GD binary here,
so there are a couple of spots where the exact API can drift between
GD/Geode versions and are worth a quick check if the build fails:

- **`FMODAudioEngine::sharedEngine()->playEffect(...)` parameter
  order.** I used `(file, pitch, pan, volume)`, which matches the
  common convention in public Geode mods, but if your SDK's
  autocomplete shows a different order, match that instead.
- **`PlayLayer::init(GJGameLevel*, bool, bool)` signature.** This has
  been stable for a while but is exactly the kind of thing that can
  shift on a GD update — if it doesn't match, your editor's Geode
  extension (or `Ctrl+Click` into the binding) will show the current
  signature.
- **`m_player2` field name.** Used to detect which player triggered
  the event; should be current, but same caveat.

None of these are structural — if one's off, it's a one-line fix, not
a redesign.

## Customizing

All the tunable bits live in `mod.json` under `"settings"` (colors,
bar size, sound on/off, volume) and show up automatically as a
settings page in Geode's mod list — no extra UI code needed for those.
