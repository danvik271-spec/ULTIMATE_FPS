# Asset source and license notes

Only general environment materials, architecture, props, vegetation, sky/lighting, ambience and interface resources are in scope for the current content pipeline.

## Poly Haven

- Official API documentation: https://polyhaven.com/our-api
- Concrete material candidate: https://polyhaven.com/a/concrete
- Concrete Floor candidate: https://polyhaven.com/a/concrete_floor
- Concrete Floor 01 candidate: https://polyhaven.com/a/concrete_floor_01
- Concrete Debris candidate: https://polyhaven.com/a/concrete_debris
- Asset license: the official pages identify the selected material candidates as CC0. The official API terms also describe the assets as CC0 and request a clear Poly Haven credit when a product integrates the live API.
- Selected concrete material page metadata: Concrete — Rob Tuytel, CC0; Concrete Floor — eye-candy.xyz, CC0; Concrete Floor 01 — Rob Tuytel, CC0; Concrete Debris — Amal Kumar, CC0.

## ambientCG

- Official site: https://ambientcg.com/
- Official API documentation: https://docs.ambientcg.com/api/
- Official license: https://docs.ambientcg.com/license/
- The publisher states that downloadable assets are provided under CC0 1.0 Universal. Verify the specific asset's page and download metadata before importing.

## Acquisition status

During repository creation, an HTTPS request from the execution container to `api.polyhaven.com` failed at DNS resolution (`Could not resolve host`). The environment did not permit downloading third-party binary assets. No file is listed as downloaded in `ASSET_MANIFEST.csv`; candidates are recorded separately in `ASSET_CANDIDATES.csv` and explicitly marked as not downloaded.

## Required provenance fields

For each acquired file, retain its source page, direct file URL, author, exact license identifier, selected resolution/format, SHA-256 and integration status. Keep any provider attribution or license text required by that asset. Do not infer an individual asset license solely from a search snippet or from another asset by the same site.
