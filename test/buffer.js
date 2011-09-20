var weak = require('../')
  , assert = require('assert')

var b = Buffer('test')
weak(b, callback)

var gotCb = false
function callback () {
  gotCb = true
  console.log(String(this))
}

b = null
gc()


process.on('exit', function () {
  assert.ok(gotCb)
})
