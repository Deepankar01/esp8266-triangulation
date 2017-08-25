const {InfluxConstants} = require("./constants");
const Influx = require('influx')

const influx = new Influx.InfluxDB({
    host: InfluxConstants.HOST,
    database: InfluxConstants.DATABASE,
    schema: [{
        measurement: InfluxConstants.MEASURMENT,
        fields: {
            RSSI: Influx.FieldType.INTEGER,
            SSID: Influx.FieldType.STRING,
            channel: Influx.FieldType.STRING,
            station: Influx.FieldType.STRING,
            SSID:Influx.FieldType.STRING
        },
        tags: [
            'routerId',
            'type',
            'BSSID',
        ]
    }]
})


const CheckDatabase = () => new Promise((resolve, reject) => {
    influx.getDatabaseNames().then(names => {
        if (!names.includes(InfluxConstants.DATABASE)) {
            return resolve(influx.createDatabase(InfluxConstants.DATABASE));
        }
        else {
            return resolve({});
        }
    }).catch((error) => reject(Error(error)));
});


const WriteData = (id, data) => new Promise((resolve, reject) => {
    influx.writePoints([{
        measurement: InfluxConstants.MEASURMENT,
        tags: {
            routerId: id,
            BSSID: data.BSSID,
            type: data.type,
        },
        fields: {
            RSSI: data.RSSI,
            station: data.STATION || "",
            channel: data.channel,
            SSID: data.SSID,
        },
    }]).then(() => {
        return resolve("Data Saved");
    })
        .catch(err => {
            return reject(Error(`Error saving data to InfluxDB! ${err.stack}`));
        })
});


module.exports = {
    CheckDatabase: CheckDatabase,
    WriteData: WriteData,
};


