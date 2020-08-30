# autoseg | Automatic Fast WebAssembly Image Segmentation

> Are you looking to convert UDT annotations into png masks? Use [udt-to-png](https://github.com/UniversalDataTool/udt-to-png)

autoseg segments images from a list of points and polygons containing
classifications. Autoseg was originally created by [Severin Ibarluzea](https://twitter.com/seveibar) for usage with the [Universal Data Tool](https://github.com/UniversalDataTool/universal-data-tool).

## Features

- Simple API
- WebWorker background computation
- Deterministic, suitable for mask compression
- Fast. WebAssembly compiled from state of the art C++ implementations of SLIC superpixeling and graph cut
- Fully compliant with the [Universal Data Tool](https://github.com/UniversalDataTool/universal-data-tool) and [UDT Format](https://github.com/UniversalDataTool/udt-format)

## Example Inputs -> Output

### Input Image

![Input Image](https://user-images.githubusercontent.com/1910070/88559148-75011400-cffa-11ea-912e-e4d53a64dc8d.png)

### Input Polygon

![Input Image Polygon](https://user-images.githubusercontent.com/1910070/88559149-7599aa80-cffa-11ea-9d7b-34f4190750f8.png)

### Output Mask

![Output Mask](https://user-images.githubusercontent.com/1910070/88559147-75011400-cffa-11ea-9ac7-d99e7bc77646.png)

## CLI

Make sure to format your file in the [\*.udt.json format](https://github.com/UniversalDataTool/udt-format).

```bash
npm install -g autoseg

# This will output a bunch of image pngs for the masks of a udt file
autoseg some_file.udt.json -o output-directory


autoseg --help

# Usage: autoseg path/to/dataset.udt.json -o output-masks-dir
#
# Options:
#   --help               Show help                                       [boolean]
#   --version            Show version number                             [boolean]
#   --use-sample-number  Use the number of the sample as the mask filename
#                       (mask0001.png, etc.)
#   --output-dir, -o     Output directory for masks                     [required]
```

## API

```javascript
const autoseg = require("autoseg") // OR require("autoseg/node")

await autoseg.loadImage({ data: imData, width: 320, height: 249 })

// Points/polygons are in the UDT Shape format
// https://github.com/UniversalDataTool/udt-format
const maskImageData = await autoseg.getMask([
  { regionType: "point", x: 50, y: 50, cls: 0 },
  { regionType: "point", x: 150, y: 150, cls: 1 },
])

// If you have a canvas, you can draw the image of the mask
canvasContext.putImageData(maskImageData, 0, 0)

// NOTE: nodejs doesn't have builtin support for ImageData, but you
// can use the returned data in a similar way, it's an object with
// { data: Uint8ClampedArray, width: number, height: number }
```

## Configuring

```javascript
await autoseg.setConfig({
  maxClusters: 1000,
  classNames: ["dog", "cat"],
  classColors: [0xffffffff, 0xff000000],
})
```
