global.fetch = require('node-fetch')
global.WebSocket = require('ws')

const telosTrxStatus = () => {
    const messageBody = {
        "apikey": "test-api-key",
        "event": "subscribe",
        "type": "get_actions",
        "data": {
            "account": "eosio",
            "actions": ["transfer"]
        }
    }
    const socket = new WebSocket("wss://testnet.telos.eostribe.io/streaming")

    socket.onopen = () => {
        socket.send(JSON.stringify(messageBody));
    }

    socket.onmessage = (event) => {
        console.log('Telos websocket:', JSON.parse(event.data))
    }
    socket.onclose = (event) => {
        console.log("Telos socket connection closed:" + event.data)
    }
    socket.onerror = function (error) {
        console.log("Telos websocket got error: " + error.message)
    }
}

telosTrxStatus()
