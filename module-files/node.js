// This require doesn't exist until this module is packaged (see Makefile)
const WASM_INIT = require("./node-wasm-bundle")
const api = require("./api")

module.exports = api(WASM_INIT)
