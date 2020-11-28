const yargs = require("yargs")
const fs = require("fs")
const download = require("download")
const image = require("get-image-data/native")
// const autoseg = require("autoseg/node")
const autoseg = require("../module/node")
const png = require("fast-png")
const mkdirp = require("mkdirp")
const cliProgress = require("cli-progress")

const { argv } = yargs
  .usage("Usage: $0 path/to/dataset.udt.json -o output-masks-dir")
  .option("use-sample-number", {
    describe:
      "Use the number of the sample as the mask filename (mask0001.png, etc.) ",
  })
  .option("output-dir", {
    alias: "o",
    describe: "Output directory for masks",
  })
  .demandOption(["o"])

const {
  _: [pathToFile],
  outputDir,
  useSampleNumber,
} = argv

async function main() {
  const ds = JSON.parse(fs.readFileSync(pathToFile))

  const config = {
    mode: "simple",
    ...ds.interface.autoSegmentationEngine,
    classNames: (ds.interface.labels[0] !== "background"
      ? ["background"]
      : []
    ).concat(
      ds.interface.labels.map((l) => (typeof l === "string" ? l : l.id))
    ),
  }

  autoseg.setConfig(config)

  await mkdirp(outputDir)

  const bar1 = new cliProgress.SingleBar({}, cliProgress.Presets.shades_classic)

  for (const [sampleIndex, { imageUrl, annotation }] of ds.samples.entries()) {
    if (sampleIndex >= 1) {
      if (sampleIndex === 1) bar1.start(ds.samples.length, 0)
      bar1.update(sampleIndex)
    }
    const fileName = imageUrl.split("/").slice(-1)[0]
    const fileBuffer = await download(imageUrl)

    const imdata = await new Promise((resolve) =>
      image(fileBuffer, (err, info) => resolve(info))
    )

    try {
      await autoseg.loadImage(imdata)
      const mask = await autoseg.getMask(annotation)
      const pngBuffer = png.encode(mask)

      fs.writeFileSync(
        `${outputDir}/${
          useSampleNumber
            ? `mask${sampleIndex.toString().padStart(6, "0")}.png`
            : fileName.split(".")[0] + ".png"
        }`,
        pngBuffer
      )
    } catch (e) {
      console.log(`error processing samples[${sampleIndex}]`, e.toString())
    }
  }
  bar1.stop()
  process.exit(0)
}

main()
