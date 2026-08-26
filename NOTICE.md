# YWD-Plug Notices and Upstream Attribution

YWD-Plug is an independent browser-based radio programming project derived from the open-source **NeonPlug** project.

## NeonPlug baseline

Upstream project: `infamy/NeonPlug`  
Repository: https://github.com/infamy/NeonPlug  
Frozen YWD baseline commit: `8ae184770e03a93959f81c262f2ba9dcb93b0400`

NeonPlug is credited as the foundation for the existing browser CPS, radio protocol implementations, shared codeplug model, editors, Web Serial/BLE integration, diagnostics, and related application architecture used by the YWD-Plug baseline.

The upstream project identifies its code as MIT licensed. Upstream copyright, attribution, and license notices must be preserved when applicable.

## DM-32UV protocol reference

Protocol reference: https://github.com/infamy/DM32-Protocol-Spec

The DM-32UV / DP570UV path is the primary hardware regression target while YWD-Plug is being modernized.

## Future DB-25D work

The YWD-Plug roadmap identifies open-source RT73 / DB-25D-family projects as possible research sources. **No GPL-derived DB-25D implementation is considered part of the baseline merely because it is listed in the roadmap.**

Before incorporating GPL-derived code or a substantial derived implementation:

- record the upstream project, author, repository, and exact source revision;
- preserve required copyright and license notices;
- identify the YWD-Plug files or algorithms that are derived from it;
- document modifications made by YWD-Plug;
- ensure distribution of the resulting work satisfies the applicable GPL terms.

See `docs/licensing.md` for the repository policy.
