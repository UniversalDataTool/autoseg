const { version } = require("./package.json")
let lastRequestId = -1

function wrapAutoseg(urlOrVersion) {
  if (urlOrVersion.match(/^[0-9]+\.[0-9]+\.[0-9]+$/)) {
    urlOrVersion = `https://unpkg.com/autoseg@${urlOrVersion}/webworker-worker-bundle.js`
  }

  const blob = new Blob([`importScripts('${urlOrVersion}')`], {
    type: "application/javascript",
  })
  const blobUrl = window.URL.createObjectURL(blob)
  const webworker = new Worker(blobUrl)

  // This is blocked by a unfixable CORS error
  // const webworker = new Worker(urlOrVersion)

  const functions = ["setConfig", "loadImage", "getMask"]

  const obj = {}

  for (const functionName of functions) {
    const id = lastRequestId++
    obj[functionName] = (...args) => {
      webworker.postMessage({ functionName, args, id })
      return new Promise((resolve, reject) => {
        const possibleResponse = (e) => {
          const { data } = e
          if (!data) return
          if (data.id === id) {
            webworker.removeEventListener("message", possibleResponse)
            if (data.error) {
              reject(new Error(data.error))
            } else {
              // There's some issue with the serialization of the ImageData
              // for getMask
              if (functionName === "getMask") {
                const { data: pixels, width, height } = data.returnValue
                resolve(new ImageData(pixels, width, height))
              } else {
                resolve(data.returnValue)
              }
            }
          }
        }
        webworker.addEventListener("message", possibleResponse)
      })
    }
  }

  return obj
}

module.exports = wrapAutoseg(
  `https://unpkg.com/autoseg@${version}/webworker-worker-bundle.js`
)
module.exports.wrapAutoseg = wrapAutoseg
