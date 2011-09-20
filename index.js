var bindings
try {
  bindings = require('./build/Release/weakref')
} catch (e) { try {
  bindings = require('./build/default/weakref')
} catch (e) {
  throw e
}}

module.exports = bindings
