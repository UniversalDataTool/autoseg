const yargs = require("yargs")
const fs = require("fs")
const download = require("download")
const image = require("get-image-data/native")
const autoseg = require("autoseg/node")
const png = require("fast-png")
const mkdirp = require("mkdirp")

const { argv } = yargs
  .usage("Usage: $0 path/to/dataset.udt.json -o output-masks-dir")
  .demandOption(["o"])

const {
  _: [pathToFile],
  o: outputDirectory,
} = argv

async function main() {
  const ds = JSON.parse(fs.readFileSync(pathToFile))

  const config = {
    classNames: ["background"].concat(
      ds.interface.labels.map((l) => (typeof l === "string" ? l : l.id))
    ),
  }

  autoseg.setConfig(config)

  await mkdirp(outputDirectory)

  for (const { imageUrl, annotation } of ds.samples) {
    // console.log({ annotation })
    const fileName = imageUrl.split("/").slice(-1)[0]
    const destPath = `/tmp/${fileName}`
    const fileBuffer = await download(imageUrl)

    const imdata = await new Promise((resolve) =>
      image(fileBuffer, (err, info) => resolve(info))
    )

    await autoseg.loadImage(imdata)
    const mask = await autoseg.getMask(annotation)
    const pngBuffer = png.encode(mask)

    fs.writeFileSync(`${outputDirectory}/${fileName}`, pngBuffer)
  }
}

main()

// const autoseg = require("./module")

// autoseg.loadImage("")
