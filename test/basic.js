var weak = require('../')
  , assert = require('assert')

var obj = {
    a: true
  , b: 'test'
  , cleanup: function () {
      console.log('cleaning up: %j', this)
      delete this.a
      delete this.b
      console.log('after cleanup: %j', this)
    }
  }

// DO NOT access the outer scope 'obj' from inside this function
var gotCb = false
function callback () {
  gotCb = true
  console.log('inside weak callback!')
  this.cleanup()
}

weak(obj, callback)
obj = null
gc()

setTimeout(function () {
  console.log('done.')
  assert.ok(gotCb)
}, 250)
