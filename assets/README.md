# README render

`opentag-prototype-cad.png` was ray-traced with KiCad CLI 10.0.1 from
[`Hardware/Hardware.kicad_pcb`](../Hardware/Hardware.kicad_pcb). No generative AI
was used for this image.

The render preserves the board geometry, copper, silkscreen, component placements,
and assigned model transforms. Model paths were resolved in a temporary copy:
`${KIPRJMOD}` points to the original Hardware directory, and legacy KiCad 8 `.wrl`
references use the same-named `.step` models in the installed KiCad 10 library.
The source board and schematic were not modified.

Render settings: 1800 × 1200 pixels, top view, rotation `325,0,30`, zoom `0.72`,
high quality, opaque background, and floor shadows; board stackup colors retained.

This is a visualization of the saved CAD state, not a complete assembly model.
The Seeed Studio XIAO footprint has no assigned model. The radio footprint uses
the board's existing DWM1000 model assignment, despite its DWM3000 electrical
labels. These historical modeling limitations are preserved rather than filled
in with invented geometry.
