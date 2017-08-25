const {InfluxConstants} = require("./constants");
const Influx = require('influx')

const influx = new Influx.InfluxDB({
    host: InfluxConstants.HOST,
    database: InfluxConstants.DATABASE,
    schema: [{
        measurement: InfluxConstants.MEASURMENT,
        fields: {
            RSSI: Influx.FieldType.INTEGER,
            channel: Influx.FieldType.INTEGER,
            type: Influx.FieldType.STRING,
            SSID: Influx.FieldType.STRING,
            MACAddress: Influx.FieldType.STRING
        },
        tags: [
            'routerId'
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
            routerId: id
        },
        fields: {
            RSSI: data.RSSI,
            channel: data.channel,
            type: data.type,
            SSID: data.SSID,
            MACAddress: data.MACAddress
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


