const { MongoConstants } = require("./constants");
const Mongoose = require("mongoose"),
    Schema = Mongoose.Schema;

// Mongoose.connect(`mongodb://${MongoConstants.HOST}/${MongoConstants.DATABASE}`,{useMongoClient: true,});

const WifiReadingSchema = new Schema({
    type: { type: String },
    RSSI: { type: Number },
    routerId: { type: Number },
    BSSID: { type: String },
    channel: { type: Number },
    SSID: { type: String },
    station: { type: String }
}, { timestamps: true })

WifiReadingModel = Mongoose.model(MongoConstants.COLLECTION, WifiReadingSchema);

const writeData = (routerId, data) => new Promise((resolve, reject) => {
    let reading = new WifiReadingModel();
    reading.type = data.type;
    reading.RSSI = data.RSSI;
    reading.routerId = routerId;
    reading.BSSID = data.BSSID;
    reading.channel = data.channel;
    reading.SSID = data.SSID;
    reading.station = data.station;

    reading.save((err) => {
        if (err) {console.log(err); return reject(err);}
        else return resolve({});
    });
})



module.exports = {
    'WriteMongoData': writeData
};