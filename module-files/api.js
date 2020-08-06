// @flow

module.exports = (WASM_INIT) => {
  let wasm = WASM_INIT()

  function isWasmLoaded() {
    return Boolean(wasm.setImageSize) // any property works
  }

  const mod = { unresolvedCalls: [] }

  async function resolveCalls() {
    while (mod.unresolvedCalls.length > 0) {
      const { f, args, resolve, reject } = mod.unresolvedCalls[0]
      mod.unresolvedCalls.shift()
      await f(...args)
        .then(resolve)
        .catch(reject)
    }
  }

  function eventuallyResolve(f) {
    return (...args) => {
      if (isWasmLoaded()) {
        return resolveCalls().then(() => {
          return f(...args)
        })
      } else {
        return new Promise((resolve, reject) => {
          mod.unresolvedCalls.push({ f, args, resolve, reject })
        })
      }
    }
  }

  let checkWasmLoadedInterval = setInterval(() => {
    if (isWasmLoaded()) {
      clearInterval(checkWasmLoadedInterval)
      resolveCalls()
    }
  }, 100)

  mod.defaultConfig = {
    mode: "autoseg",
    maxClusters: 1000,
    classColors: [
      0x88000000,
      2285257716,
      2297665057,
      2286989132,
      2281729263,
      2286441849,
      2285412200,
      2288197353,
      2293245852,
      2293584191,
      2290652672,
      2285493453,
      2290842976,
    ],
    classNames: [],
  }
  mod.config = { ...mod.defaultConfig }

  mod.setConfig = eventuallyResolve(async (config) => {
    mod.config = { ...mod.defaultConfig, ...config }
  })
  mod.loadImage = eventuallyResolve(async (imageData) => {
    wasm.setSimpleMode(mod.config.mode === "simple")
    wasm.setMaxClusters(mod.config.maxClusters)
    wasm.setImageSize(imageData.width, imageData.height)
    mod.imageSize = { width: imageData.width, height: imageData.height }
    for (let i = 0; i < mod.config.classColors.length; i++) {
      wasm.setClassColor(i, mod.config.classColors[i])
    }
    const imageAddress = wasm.getImageAddr()
    wasm.HEAPU8.set(imageData.data, imageAddress)
    wasm.computeSuperPixels()
    mod.imageLoaded = true
  })
  mod.getMask = eventuallyResolve(async (objects) => {
    wasm.clearClassElements()
    const { width, height } = mod.imageSize
    // convert bounding boxes to polygons
    objects = objects.map((r) => {
      if (r.regionType !== "bounding-box") return r
      return {
        regionType: "polygon",
        classification: r.classification,
        points: [
          { x: r.centerX - r.width / 2, y: r.centerY - r.height / 2 },
          { x: r.centerX + r.width / 2, y: r.centerY - r.height / 2 },
          { x: r.centerX + r.width / 2, y: r.centerY + r.height / 2 },
          { x: r.centerX - r.width / 2, y: r.centerY + r.height / 2 },
        ],
      }
    })

    const clampX = (x) => (x > width ? width - 1 : x < 0 ? 0 : x)
    const clampY = (y) => (y > height ? height - 1 : y < 0 ? 0 : y)

    for (let object of objects) {
      const clsIndex =
        typeof object.classification === "number"
          ? object.classification
          : mod.config.classNames.indexOf(object.classification)
      if (clsIndex > mod.config.classColors.length || clsIndex === -1) {
        continue
      }

      switch (object.regionType) {
        case "polygon": {
          const { points } = object
          const pointPairs = points.map((p, i) => [
            p,
            points[(i + 1) % points.length],
          ])
          const linesToAdd = []
          for (const [p1, p2] of pointPairs) {
            const ri1 = clampY(Math.round(p1.y * height))
            const ci1 = clampX(Math.round(p1.x * width))
            const ri2 = clampY(Math.round(p2.y * height))
            const ci2 = clampX(Math.round(p2.x * width))
            if (ri1 === ri2 && ci1 === ci2) continue
            linesToAdd.push({ ri1, ci1, ri2, ci2 })
          }
          if (linesToAdd.length >= 3) {
            const pi = wasm.addPolygon(clsIndex)
            for (const { ri1, ci1, ri2, ci2 } of linesToAdd) {
              wasm.addLineToPolygon(pi, ri1, ci1, ri2, ci2)
            }
          }
          break
        }
        case "point": {
          const { x, y } = object
          if (x < 0 || x >= 1) continue
          if (y < 0 || y >= 1) continue

          wasm.addClassPoint(
            clsIndex,
            clampY(Math.floor(y * mod.imageSize.height)),
            clampX(Math.floor(x * mod.imageSize.width))
          )
          break
        }
        default: {
          continue
        }
      }
    }
    wasm.computeMasks()
    const maskAddress = wasm.getColoredMask()
    const cppImDataUint8 = new Uint8ClampedArray(
      wasm.HEAPU8.buffer,
      maskAddress,
      width * height * 4
    )

    if (typeof ImageData !== "undefined") {
      // Browser
      return new ImageData(cppImDataUint8, width, height)
    } else {
      // NodeJS
      return { data: cppImDataUint8, width, height }
    }
  })

  return mod
}
