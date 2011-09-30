var bindings
try {
  bindings = require('./build/Release/weakref')
} catch (e) { try {
  bindings = require('./build/default/weakref')
} catch (e) {
  throw e
}}

module.exports = bindings.create

Object.keys(bindings).forEach(function (name) {
  module.exports[name] = bindings[name]
})
