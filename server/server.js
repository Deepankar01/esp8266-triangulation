const PORT = 8080;
var restify = require("restify");
var server = restify.createServer();
server.use(restify.plugins.bodyParser());
const { SaveToDatabase } = require("./utils/SaveToDatabase");

//id is the unique id of the server


const saveTheAwsomeness = (req, res, next) => {
  const routerId = req.params.id;
  const data = req.body;
  SaveToDatabase(routerId, data)
  .then((status) => res.send(200, status))
  .catch((errorStatus) => res.send(500, errorStatus)) 
}

server.post('/:id', saveTheAwsomeness);

server.listen(PORT, function () {
  console.log('%s listening at %s', server.name, server.url);
});