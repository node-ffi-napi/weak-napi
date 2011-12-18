require('should')
var weak = require('../')

describe('weak()', function () {

  describe('Buffer', function () {

    beforeEach(gc)

    it('should invoke callback before destroying Buffer', function () {

      var called = false
      weak(Buffer('test'), function (buf) {
        called = true
        buf.toString().should.equal('test')
      })

      called.should.be.false
      gc()
      called.should.be.true
    })

  })
})
