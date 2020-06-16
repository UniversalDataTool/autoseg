const test = require("ava")
const fs = require("fs")
const path = require("path")
const m = require("../../build/module.js")

const f = (p) => path.join(__dirname, p)

const imData = Uint8Array.from(fs.readFileSync(f("../../assets/orange.bin")))

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

  m.clearClassPoints()
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

  fs.writeFileSync(f("./mask.bin"), cppImDataUint8)
})
