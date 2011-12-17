var bindings

try {
  bindings = require('./compiled/'+process.platform+'/'+process.arch+'/weakref')
} catch (e) { try {
  bindings = require('./build/Release/weakref')
} catch (e) { try {
  bindings = require('./build/default/weakref')
} catch (e) {
  throw e
}}}

module.exports = bindings.create

// backwards-compat with node-weakref
bindings.weaken = bindings.create

Object.keys(bindings).forEach(function (name) {
  module.exports[name] = bindings[name]
})
