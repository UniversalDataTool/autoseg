const qe = require("./build/module.js")

function tryToInit() {
  if (!qe.setImage) return setTimeout(tryToInit, 100)

  const image = Uint8Array.from([[[255, 255, 255, 255]]])

  qe.setImage(image, 1, 1)
  qe.addClassPoint(10, 10, 0)
  qe.addClassPoint(2, 15, 1)

  // qe.setClassPoints([10, 18], [10, 2], [0, 0])

  console.log("done")
}

setTimeout(tryToInit, 100)
