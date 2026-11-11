# Style sheet (measured from FACE_NORMAL)

1. Eye shape: tall rounded rectangle, bulging top/bottom, tapered corners
2. Outline: none (solid shape)
3. Interior: fully filled, no pupil
4. Eye width: 24 px (x 20-43 / 84-107)
5. Eye height: 37 px (rows 13-49)
6. Eye centre row: 31
7. Gap between eyes: 40 px
8. Mouth: filled crescent smile with two raised corner tufts
9. Mouth width: 23 px (x 52-74)
10. Mouth centre row: ~50 (rows 46-54)
11. Mouth line weight: filled shape (~5 rows deep)
12. Highlights: none
13. Cheeks: none
14. Eyebrows: none
15. Margins: L20 R20 T13 B9 px
16. Silhouette: no face outline; features float on black

# Notes on this release
* Bitmaps are horizontal-format (MSB first, 8 horizontal pixels per byte), as Adafruit drawBitmap expects.
* Repaired faces (blank/duplicate row removed or added, verified pixel-exact against images/*.png):
  love (-row 53), angry (-row 53), kiss (-row 24), sad (-row 26),
  sleepy (+row 34), dizzy (+row 53), hot (+row 59).
* pooker: had 63 rows and does NOT match its PNG; a blank row was appended so it is 1024 bytes
  (no out-of-bounds read). The art itself still needs a manual check.
* Closed eyes in blink/sneeze are 3 px strokes (same weight as wink/sleepy lines), same x-span and centre row as FACE_NORMAL.
* The "1-8 bytes per frame" rule is physically impossible for eye/mouth geometry in a horizontal-format
  bitmap (one eye row = 3 bytes); real diffs are listed in ANIMATION_DIFFS.md.
* look_left / look_right have 3 frames (normal, shifted, normal) so they end on an existing face.
