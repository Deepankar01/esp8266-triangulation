
const constants = require("./constants");
const { CheckDatabase, WriteData } = require("./InfluxHelpers");

const ValidateData = (data) => new Promise((resolve, reject) => (
    data.hasOwnProperty("RSSI") && data.hasOwnProperty("channel") 
    && data.hasOwnProperty("type") && data.hasOwnProperty("SSID") 
    && data.hasOwnProperty("BSSID")) ? resolve({}) : reject("invalid property")
)


const SaveToDatabase = (routerId, dataReceived) => new Promise((resolve, reject) => {
    console.log("%s id =======> %s", routerId, JSON.stringify(dataReceived));
    ValidateData(dataReceived).then(() => CheckDatabase())
        .then(() => WriteData(routerId, dataReceived))
        .then((response) => resolve(constants.OKResponse))
        .catch((err) => reject(constants.ERRORResponse));
});

module.exports = {
    'SaveToDatabase': SaveToDatabase
}
