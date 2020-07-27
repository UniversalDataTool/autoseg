const test = require("ava")
const fs = require("fs")
const path = require("path")
const autoseg = require("../../module/node")

const f = (p) => path.join(__dirname, p)

const imData = Uint8Array.from(
  fs.readFileSync(f("../../assets/orange-320x249.bin"))
)

test("use api to load with built module (nodejs)", async (t) => {
  // console.log(autoseg)
  // console.log(imData.length)
  await autoseg.loadImage({ data: imData, width: 320, height: 249 })

  const result = await autoseg.getMask([
    { regionType: "point", x: 50, y: 50, cls: 0 },
    { regionType: "point", x: 150, y: 150, cls: 1 },
  ])

  t.assert(result)
  t.assert(result.data.length === 320 * 249 * 4)
})
