module.exports = {
    OKResponse: "OK",
    ERRORResponse: "ERROR",
    InfluxConstants :{
        HOST:"influx-database",
        DATABASE:"esp8266-data",
        MEASURMENT:"network_environment",
    },
    MongoConstants:{
        HOST: "mongodb-container",
        DATABASE: "esp8266-network-scan",
        COLLECTION: "scan",
    }
}