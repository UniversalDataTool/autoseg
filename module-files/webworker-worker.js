// I found this guide to be helpful -Sev
// https://www.kevinhoyt.com/2018/10/23/image-processing-in-a-web-worker/

import "regenerator-runtime/runtime"

// This require doesn't exist until this module is packaged (see Makefile)
const WASM_INIT = require("./node-wasm-bundle")
const api = require("./api")

const wasm = api(WASM_INIT)

self.addEventListener("message", async (e) => {
  const { functionName, varName, args, id } = e.data
  if (varName) {
    self.postMessage({ varName, varVal: wasm[varName], id })
    return
  }
  try {
    const res = {
      functionName,
      returnValue: await wasm[functionName](...args),
      id,
    }
    if (functionName === "getMask") {
      self.postMessage({
        functionName,
        id,
        // ImageData doesn't transfer properly... not sure why -Sev
        returnValue: {
          data: new Uint8ClampedArray(res.returnValue.data),
          width: res.returnValue.width,
          height: res.returnValue.height,
        },
      })
    } else {
      self.postMessage(res)
    }
  } catch (e) {
    console.log(`error calling ${functionName}: ${e.toString()}`)
    self.postMessage({
      functionName,
      error: e.toString(),
      id,
    })
  }
})
