var evilscan = require('evilscan');

var options = {
    target:'192.168.1.0/24',
    port:'11861',
    status:'O', // Timeout, Refused, Open, Unreachable
    banner:true
};

var scanner = new evilscan(options);

scanner.on('result',function(data) {
    // fired when item is matching options
        console.log(data);
});

scanner.on('error',function(err) {
    throw new Error(data.toString());
});

scanner.on('done',function() {
    // finished !
});

scanner.run();