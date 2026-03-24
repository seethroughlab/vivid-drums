# vivid-drums (Archived)

> **These operators are now part of [Vivid core](https://github.com/seethroughlab/vivid).** This repository is archived and no longer maintained.

`vivid-drums` was a Vivid package library that provided drum synthesis operators.
It now also shipped drum-stack macro examples/presets for fast layered kit starting points.

## Preview

![vivid-drums preview](docs/images/preview.png)

## Local install

```bash
./build/vivid link ../vivid-drums
./build/vivid rebuild vivid-drums
```

## Package contents

- `src/drum_kick.cpp`
- `src/drum_snare.cpp`
- `src/drum_hihat.cpp`
- `src/drum_clap.cpp`
- `src/drum_cymbal.cpp`
- `src/drum_tom.cpp`
- `graphs/drum_stack_foundation.json`
- `graphs/drum_stack_percussion_wash.json`
- `factory_presets/*.json` (includes `Stack Foundation ...` macro presets)
- `vivid-package.json`


## Validation

Before pushing changes:

1. Configure + build package operators.
2. Run package tests.
3. Run `vivid` link/rebuild/uninstall cycle against this package.
4. Run `test_demo_graphs` against this package's `graphs/` directory.
5. Keep the drum-stack asset regression test green alongside graph smoke.
