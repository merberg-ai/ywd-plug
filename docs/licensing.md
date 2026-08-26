# Licensing and Upstream Policy

## Baseline license

The YWD-Plug baseline is distributed under the MIT License. It is derived from the NeonPlug project, whose upstream project documentation identifies it as MIT licensed.

Frozen upstream baseline:

```text
https://github.com/infamy/NeonPlug
8ae184770e03a93959f81c262f2ba9dcb93b0400
```

YWD-Plug must preserve applicable upstream notices rather than erasing authorship during rebranding.

## GPL-derived implementation

MIT is GPL-compatible, but that does not make GPL-derived code MIT code. If YWD-Plug later incorporates GPLv3-covered source or a derivative implementation from projects such as RT73 utilities or related DB-25D tooling, the GPL-covered portions and the distributed combined work must be handled in accordance with that upstream license.

Before importing such work, add a source-specific note under `docs/upstream/` that records:

- upstream project and author(s)
- repository URL
- exact commit/tag/release used
- upstream license
- files or algorithms adapted
- modifications made for YWD-Plug
- any required build/source distribution notes

Do this **before** or in the same commit that imports derived implementation code, never as later cleanup.

## External data sources

Radio/contact/repeater/aviation data services referenced by the application are external data sources, not automatically part of the YWD-Plug source license. Their own terms, attribution requirements, and redistribution rules still apply.

## Test fixtures and vendor files

Do not commit vendor firmware or vendor-generated binary codeplug images unless redistribution is clearly permitted. Prefer sanitized fixtures that can legally be redistributed. Keep private/local hardware fixtures outside Git when their status is uncertain.
