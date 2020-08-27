const HyperionSocketClient = require('@eosrio/hyperion-stream-client').default
const { Document } = require('./model')
const { DGraph } = require('./service')

async function run () {
  const dgraph = new DGraph({})
  const document = new Document(dgraph)

  await document.setSchema()

  const ENDPOINT = 'https://testnet.telos.caleos.io'
  const client = new HyperionSocketClient(ENDPOINT, { async: true })

  client.onConnect = () => {
    client.streamDeltas({
      code: 'docs.hypha',
      table: 'documents',
      account: 'docs.hypha',
      scope: '',
      payer: '',
      start_from: '2020-08-15T00:00:00.000Z',
      read_until: 0
    })
  }

  // see 3 for handling data
  client.onData = async (delta, ack) => {
    const {
      content: { data: doc }
    } = delta
    console.log(JSON.stringify(doc, null, 4))
    if (doc) {
      await document.store(doc)
      ack()
    }
  }

  client.connect(() => {
    console.log('connected!')
  })
}

run()
