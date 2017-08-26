const PORT = 8080;
var restify = require("restify");
var server = restify.createServer();
const { MongoConstants } = require("./utils/constants");
const { SaveToDatabase } = require("./utils/SaveToDatabase");
const Mongoose = require("mongoose");

Mongoose.connect(`mongodb://${MongoConstants.HOST}/${MongoConstants.DATABASE}`, { useMongoClient: true, });
server.use(restify.plugins.bodyParser());

//id is the unique id of the server

const saveTheAwsomeness = (req, res, next) => {
  const routerId = req.params.id;
  const data = JSON.parse(JSON.stringify(req.body).replace(/"/gm, "").replace(/:}/gm, "}").replace(/'/gm, "\""));

  SaveToDatabase(routerId, data)
    .then((status) => res.send(200, status))
    .catch((errorStatus) => res.send(500, errorStatus))
}

server.post('/:id', saveTheAwsomeness);

server.listen(PORT, function () {
  console.log('%s listening at %s', server.name, server.url);
});