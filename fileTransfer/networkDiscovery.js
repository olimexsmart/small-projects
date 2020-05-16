

require('dns').reverse('192.168.1.149', function(err, domains) {
    console.log(domains);
});