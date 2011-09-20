var weak = require('../')

var o = { 'foo': 'bar' }

weak(o, function () {
  console.log('first')
  console.log(this)
})
weak(o, function () {
  console.log('second')
  console.log(this)
})

o = null
gc()
