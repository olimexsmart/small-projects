var fs = require('fs');
var crypto = require('crypto');

fs.readFile('/home/olli/Downloads/LANsend/bomber.mp4', function(err, data) {
    console.log(err)
  var checksum = generateChecksum(data);
  console.log(checksum);
});

function generateChecksum(str, algorithm, encoding) {
    return crypto
        .createHash(algorithm || 'sha256')
        .update(str, 'utf8')
        .digest(encoding || 'hex');
}