const http = require('http');
qs = require('querystring');

var server = http.createServer(function(req, res) {
  if (req.method === 'POST') {
    var body = '';
    req.on('data', function(chunk) {
      body += chunk;
    });
    req.on('end', function() {
      var data = qs.parse(body);
      // now you can access `data.email` and `data.password`
      res.writeHead(200);
      console.log(data);
      res.end("OK");
    });
  } else {
    res.writeHead(404);
    res.end();
  }
});

server.listen(8000);