# Recovery tooling

This directory reconstructs the first maintainable YWD-Plug source baseline from the surviving `0.0.3-alpha1` deployment archive plus the pinned NeonPlug upstream source.

The recovery is intentionally conservative:

1. clone exactly the commit in `UPSTREAM_COMMIT`;
2. copy the real TypeScript/React source and tests;
3. apply YWD naming, storage namespace, `.ywdplug` export compatibility, and the YWD-Hotspot design-token layer;
4. prove `src/radios/dm32uv/` was not modified;
5. run unit, production-build, and single-file-build gates.

The GitHub recovery workflow uses these same gates before it is allowed to commit the reconstructed source to `dev`.

Once the source recovery has landed and hardware regression passes, this machinery can be retired to historical documentation rather than becoming a permanent build dependency.
