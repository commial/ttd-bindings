# Changelog

All notable changes to this project will be documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.0.0/),
and this project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

## [0.2] - 2022-07-17

### Thanks

- [Sylvain Peyrefitte](https://github.com/citronneur)

### Added/Changed

- Add support for:
    - threads
    - events (thread create and terminate, module load and unload)
    - x86_64 full context
    - exceptions
    - memory watchpoints
- Add corresponding Python wrappers
- CI with compilation checks
- Position:
    - comparison
    - MAX constant
- New examples:
    - `example_cov`: module coverage computation
    - `example_tenet`: Tenet compatible trace production

### Fixed

- Remove `/MT` from compilation chain

## [0.1] - 2022-03-21

### Added/Changed

- Initial publication
