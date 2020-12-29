const { Api, JsonRpc } = require('eosjs')
const { JsSignatureProvider } = require('eosjs/dist/eosjs-jssig')
const fetch = require('node-fetch')
const { TextEncoder, TextDecoder } = require('util')
const commandLineArgs = require('command-line-args')
const commandLineUsage = require('command-line-usage')
const fs = require('fs')

async function getDocument (host, contract, dochash) {
  let rpc
  const options = {}

  rpc = new JsonRpc(host, { fetch })
  options.code = contract
  options.json = true
  options.scope = contract
  options.table = 'documents'
  options.index_position = 2
  options.key_type = 'sha256'
  // options.encode_type = "hex";
  options.upper_bound = dochash
  options.lower_bound = dochash
  options.limit = 1

  const result = await rpc.get_table_rows(options)
  if (result.rows.length > 0) {
    return result.rows[0]
  }

  console.log('There is no document with hash: ', dochash)
  return undefined
}

async function getDocuments (host, contract) {
  let rpc
  const options = {}

  rpc = new JsonRpc(host, { fetch })
  options.code = contract
  options.json = true
  options.scope = contract
  options.table = 'documents'
  options.limit = 1000
console.log (options)
  const result = await rpc.get_table_rows(options)
  if (result.rows.length > 0) {
    return result.rows
  }

  console.log('There are no documents')
  return undefined
}

async function sendtrx (prod, host, contract, action, authorizer, data) {
  const rpc = new JsonRpc(host, { fetch })
  var defaultPrivateKey
  // key for johnnyhypha1, etc: 5HwnoWBuuRmNdcqwBzd1LABFRKnTk2RY2kUMYKkZfF8tKodubtK
  if (prod) { defaultPrivateKey = process.env.PRIVATE_KEY } else defaultPrivateKey = '5HwnoWBuuRmNdcqwBzd1LABFRKnTk2RY2kUMYKkZfF8tKodubtK' //'5KeSkosYfKj8yQRRvfWsTubTJrhBvEZixFYN8z7CfQmxf5nrjsY'
  const signatureProvider = new JsSignatureProvider([defaultPrivateKey])
  // console.log (defaultPrivateKey)
  // console.log(JSON.stringify(data, null, 2))
  const api = new Api({ rpc, signatureProvider, textDecoder: new TextDecoder(), textEncoder: new TextEncoder() })
  const actions = [{ account: contract, name: action, authorization: [{ actor: authorizer, permission: 'active' }], data: data }]
  // console.log(JSON.stringify(actions, null, 2));
  const result = await api.transact({ actions: actions }, { blocksBehind: 3, expireSeconds: 30 })
  console.log('Transaction Successfull : ', result.transaction_id)
}

class FileDetails {
  constructor (filename) {
    this.filename = filename
    this.exists = fs.existsSync(filename)
  }
}

async function loadOptions () {
  const optionDefinitions = [
    { name: 'file', alias: 'f', type: filename => new FileDetails(filename) },
    { name: 'host', alias: 'h', type: String, defaultValue: 'https://testnet.telos.caleos.io' },
    { name: 'contract', type: String, defaultValue: 'dao1.hypha' },
    { name: 'hash', type: String },
    { name: 'from', type: String },
    { name: 'to', type: String },
    { name: 'edge', type: String },
    { name: 'create', type: Boolean, defaultValue: false },
    { name: 'getorcreate', type: Boolean, defaultValue: false },
    { name: 'link', type: Boolean, defaultValue: false },
    { name: 'fork', type: Boolean, defaultValue: false },
    { name: 'certify', type: String },
    { name: 'json', type: Boolean },
    { name: 'get', type: Boolean, defaultValue: false },
    { name: 'getall', type: Boolean, defaultValue: false },
    { name: 'auth', type: String }]
  // see here to add new options:
  //   - https://github.com/75lb/command-line-args/blob/master/doc/option-definition.md

  return commandLineArgs(optionDefinitions)
}

const sections = [
  {
    header: 'Document graph',
    content: 'Create, fork, certify and query documents'
  },
  {
    header: 'Options',
    optionList: [
      {
        name: 'create',
        description: 'create a new document?',
        type: Boolean
      },
      {
        name: 'link',
        description: 'link one document to another document with an edge name',
        type: Boolean
      },
      {
        name: 'file',
        description: 'relative path to the json file containing the document',
        alias: 'f',
        type: String
      },
      {
        name: 'fork',
        description: 'fork an existing document?',
        type: Boolean
      },
      {
        name: 'hash',
        typeLabel: '{underline hash}',
        description: 'hash of the document to be forked',
        type: String
      },
      {
        name: 'certify',
        typeLabel: '{underline hash}',
        description: 'certify an existing document',
        type: String
      },
      {
        name: 'get',
        description: 'retrieve and print the document for the hash provided',
        type: Boolean
      },
      {
        name: 'getall',
        description: 'retrieve and print all documents',
        type: Boolean
      },
      {
        name: 'auth',
        typeLabel: '{underline account}',
        description: 'account to use when creating, forking, or certifying documents (e.g. johnnyhypha1)',
        type: String
      },
      {
        name: 'host',
        description: 'eosio endpoint, defaults to https://test.telos.kitchen',
        alias: 'h',
        type: String
      },
      {
        name: 'contract',
        description: 'contract to use, defaults to docs.hypha',
        type: String
      },
      {
        name: 'json',
        description: 'output in JSON',
        type: Boolean
      },
      {
        name: 'help',
        description: 'Print this usage guide.'
      }
    ]
  },
  {
    header: 'Examples',
    content: [
      '$ node index.js --host https://test.telos.kitchen --get --hash d4a2cd2e9e4eab783c12bd5aa279036b724fca0e71a064b4030c21a7466e0289',
      '$ node index.js --host https://test.telos.kitchen --create -f ../test/examples/lorem.json --auth johnnyhypha1'
    ]
  }
]

const main = async () => {
  const opts = await loadOptions()

  if (opts.create) {
    const doc = JSON.parse(fs.readFileSync(opts.file.filename, 'utf8'))
    doc.creator = opts.auth
    await sendtrx(0, opts.host, opts.contract, 'create', opts.auth, doc)
  }

  if (opts.getorcreate) {
    const doc = JSON.parse(fs.readFileSync(opts.file.filename, 'utf8'))
    doc.creator = opts.auth
    await sendtrx(0, opts.host, opts.contract, 'getorcreate', opts.auth, doc)
  }

  if (opts.fork) {
    var doc = JSON.parse(fs.readFileSync(opts.file.filename, 'utf8'))
    doc.hash = opts.hash
    await sendtrx(0, opts.host, opts.contract, 'fork', opts.auth, doc)
  }

  if (opts.certify) {
    const data = { certifier: opts.auth, hash: opts.certify, notes: 'Certified from CLI' }
    await sendtrx(0, opts.host, opts.contract, 'certify', opts.auth, data)
  }

  if (opts.get) {
    if (opts.json) {
      const singleDoc = await getDocument(opts.host, opts.contract, opts.hash)
      console.log('Single document: ', JSON.stringify(singleDoc, null, 2))
    } else {
      const singleDoc = await getDocument(opts.host, opts.contract, opts.hash)
      console.log('Single document: ', singleDoc)
    }
  }

  if (opts.link) {
    const data = { from_node: opts.from, to_node: opts.to, edge_name: opts.edge }
    console.log (data)
    await sendtrx(0, opts.host, opts.contract, 'newedge', opts.auth, data)
  }

  if (opts.getall) {
    const docs = await getDocuments(opts.host, opts.contract)
    if (opts.json) {
      console.log('Documents table: ', JSON.stringify(docs, null, 2))
    } else {
      console.log('Documents table: ', docs)
    }
  }

  if (opts.help || (!opts.getall && !opts.get && !opts.create && !opts.fork && !opts.certify && !opts.link && !opts.getorcreate)) {
    const usage = commandLineUsage(sections)
    console.log(usage)
  }
}

main()


// e91c036d9f90a9f2dc7ab9767ea4aa19c384431a24e45cf109b4fded0608ec99
// c0b0e48a9cd1b73ac924cf58a430abd5d3091ca7cbcda6caf5b7e7cebb379327