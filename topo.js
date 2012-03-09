#!/usr/local/bin/node

try {
        var handle = require('./build/Release/disktopo') ;
        var disktopo = new handle.topohandle();
        console.log(disktopo.version()) ;
        var listObj = disktopo.list();
        console.log(JSON.stringify(listObj)) ;
} catch (err) {
        console.log(err) ;
}

