# autoseg | Automatic Fast WebAssembly Image Segmentation

autoseg segments images from a list of points and polygons containing
classifications. Autoseg was originally created by [Severin Ibarluzea](https://twitter.com/seveibar) for usage with the [Universal Data Tool](https://github.com/UniversalDataTool/universal-data-tool).

## Features

- Simple API
- WebWorker background computation
- Deterministic, suitable for mask compression
- Fast. WebAssembly compiled from state of the art implementations of SLIC superpixeling and graph cut
- Fully compliant with the [Universal Data Tool](https://github.com/UniversalDataTool/universal-data-tool) and [UDT Format](https://github.com/UniversalDataTool/udt-format)

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
})
```
