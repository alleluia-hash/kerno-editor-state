# Kerno's Editor State

A Geode mod for Geometry Dash that adds a **State** button to the level editor. Click it to see
the vehicle, speed, gravity, size and mirror state at the first Start Position, and the timeline
of portal changes leading up to it. The timeline is also written to the log.

## Building

Install the [Geode SDK](https://geode-sdk.org/) and use the `geode` CLI:

```
geode build
```

Or set `GEODE_SDK` to the SDK folder and build with CMake. Targets: Windows 64-bit and Android
64-bit (see `.github/workflows/build.yml`).

## Known unknowns

The mod reads level-editor internals that were written from memory and never checked against a
running game. If the readout is wrong or the build fails, start with these:

- `LevelEditorLayer::m_objects` and the Start Position object ID (`31`).
- `LevelEditorLayer::m_levelSettings` and its `m_startSpeed` / `m_startMode` / `m_startMini` /
  `m_isFlipped` / `m_mirrorMode` fields.
- The portal object IDs in `speedIndex` / `modeIndex`, and the gravity (`10`/`11`), size
  (`99`/`101`) and mirror (`45`/`46`) IDs in `onReadout`. Several are marked "verify" in the
  source for a reason.
- The button position, which currently sits at 10% of the screen width and height and is a guess.

## Licence

MIT, see `LICENSE`.
