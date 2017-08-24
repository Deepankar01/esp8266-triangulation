
const constants = require("./constants");

const SaveToDatabase = (routerId, dataReceived) => new Promise((resolve, reject) => {

    console.log("%s id =======> %s", routerId, JSON.stringify(dataReceived));
    return resolve(constants.OKResponse);

    return reject(constants.ERRORResponse);
});

module.exports = {
    'SaveToDatabase': SaveToDatabase
}
