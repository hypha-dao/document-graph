const HyperionSocketClient = require('@eosrio/hyperion-stream-client').default;

const ENDPOINT = "https://testnet.telos.caleos.io";
const client = new HyperionSocketClient(ENDPOINT, { async: false });

client.onConnect = () => {
    client.streamActions({
        contract: 'publsh.hypha',
        action: 'event',
        account: 'publsh.hypha',
        start_from: '2020-08-15T00:00:00.000Z',
        read_until: 0,
        filters: [],
    });
}

// see 3 for handling data
client.onData = async (data, ack) => {
    console.log ("A new document has been created:");
    const document = JSON.stringify(data.content.act.data, null, 2);
    console.log(document); // process incoming data, replace with your code    
}

client.connect(() => {
    console.log('connected!');
});