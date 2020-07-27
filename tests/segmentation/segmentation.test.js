const test = require("ava")
const fs = require("fs")
const path = require("path")
const m = require("../../build/module-wasm-bundle.js")
const range = require("lodash/range")

const f = (p) => path.join(__dirname, p)

const imData = Uint8Array.from(
  fs.readFileSync(f("../../assets/orange-320x249.bin"))
)

async function setup() {
  while (!m.setImageSize) {
    await new Promise((resolve) => setTimeout(resolve, 10))
  }
}

test("should be able to set the image and call some methods", async (t) => {
  await setup()
  m.setImageSize(320, 249)
  const imageAddress = m.getImageAddr()
  m.HEAPU8.set(imData, imageAddress)

  m.setClassColor(0, 0xffffffff)
  m.setClassColor(1, 0xff0000ff)
  m.addClassPoint(0, 100, 125)
  m.addClassPoint(1, 10, 10)
  m.addClassPoint(1, 240, 300)
  t.assert(imData[0] === (m.getValue(imageAddress, "i8") & 0xff))
  t.assert(imData[1] === (m.getValue(imageAddress + 1, "i8") & 0xff))
  t.assert(imData[2] === (m.getValue(imageAddress + 2, "i8") & 0xff))

  t.assert(imData[0] === (m.HEAP8[imageAddress] & 0xff))
  t.assert(imData[1] === (m.HEAP8[imageAddress + 1] & 0xff))
  t.assert(imData[2] === (m.HEAP8[imageAddress + 2] & 0xff))

  const cppImData = m.HEAP32.slice(
    imageAddress >> 2,
    (imageAddress >> 2) + imData.length / 4
  )

  t.assert(imData[0] === (cppImData[0] & 0xff))
  t.assert(imData[1] === ((cppImData[0] >> 8) & 0xff))
  t.assert(imData[2] === ((cppImData[0] >> 16) & 0xff))
  t.assert(imData[3] === ((cppImData[0] >> 24) & 0xff))

  const cppImDataUint8 = new Uint8Array(
    m.HEAPU8.buffer,
    imageAddress,
    imData.length
  )

  t.assert(cppImDataUint8[0] === imData[0])
  t.assert(cppImDataUint8[1] === imData[1])
  t.assert(cppImDataUint8[2] === imData[2])
  t.assert(cppImDataUint8[3] === imData[3])
})

test("should be able to get a mask", async (t) => {
  await setup()
  m.setVerboseMode(true)
  m.setImageSize(320, 249)
  const imageAddress = m.getImageAddr()
  m.HEAPU8.set(imData, imageAddress)

  m.clearClassElements()
  m.setClassColor(0, 0xff0000ff)
  m.setClassColor(1, 0xffffffff)
  m.addClassPoint(0, 100, 125)
  m.addClassPoint(0, 100, 150)
  m.addClassPoint(0, 173, 108)
  m.addClassPoint(0, 180, 108)
  m.addClassPoint(1, 40, 280)
  m.addClassPoint(1, 10, 10)
  m.addClassPoint(1, 240, 300)
  m.addClassPoint(1, 200, 100)
  m.addClassPoint(1, 180, 85)
  m.computeSuperPixels()
  m.computeMasks()
  const maskAddress = m.getColoredMask()
  const cppImDataUint8 = new Uint8ClampedArray(
    m.HEAPU8.buffer,
    maskAddress,
    imData.length
  )

  t.assert(cppImDataUint8[0] === 255)

  fs.writeFileSync(f("./mask-320x249.bin"), cppImDataUint8)
})

test("should be able to get a simple mode polygon mask", async (t) => {
  await setup()
  m.setVerboseMode(true)
  m.setImageSize(320, 249)
  m.setSimpleMode(true)
  const imageAddress = m.getImageAddr()
  m.HEAPU8.set(imData, imageAddress)

  m.clearClassElements()
  m.setClassColor(0, 0xff0000ff)
  m.setClassColor(1, 0xffff00ff)

  const pi1 = m.addPolygon(0)
  const pi2 = m.addPolygon(1)
  const pi3 = m.addPolygon(1)

  m.addLineToPolygon(pi1, 0, 0, 249, 40)
  m.addLineToPolygon(pi1, 249, 40, 249, 0)
  m.addLineToPolygon(pi1, 249, 0, 0, 0)

  m.addLineToPolygon(pi2, 80, 117, 144, 180)
  m.addLineToPolygon(pi2, 144, 180, 80, 180)
  m.addLineToPolygon(pi2, 80, 180, 80, 117)

  const points = range(7).map((i) => [
    100 + Math.sin((i / 7) * Math.PI * 2) * 80,
    100 + Math.cos((i / 7) * Math.PI * 2) * 80,
  ])
  const pairs = points.map((p, i) => [p, points[(i + 1) % points.length]])
  for (const [p1, p2] of pairs) {
    m.addLineToPolygon(pi3, p1[0], p1[1], p2[0], p2[1])
  }

  m.computeSuperPixels()
  m.computeMasks()
  const maskAddress = m.getColoredMask()
  const cppImDataUint8 = new Uint8ClampedArray(
    m.HEAPU8.buffer,
    maskAddress,
    imData.length
  )

  t.pass("TODO for now manually confirm mask content in mask-polygon-only.bin!")

  fs.writeFileSync(f("./mask-320x249-polygon-only.bin"), cppImDataUint8)
})
