const { Api, JsonRpc } = require("eosjs");
const { JsSignatureProvider } = require("eosjs/dist/eosjs-jssig");
const fetch = require("node-fetch");
const { TextEncoder, TextDecoder } = require("util");

const fs = require('fs');

async function getDocument (host, contract, dochash) {
    let rpc;
    let options = {};

    rpc = new JsonRpc(host, { fetch });
    options.code = contract;
    options.json = true;
    options.scope = contract;
    options.table = "documents";
    options.index = 2;
    options.key_type = "sha256";
    // options.encode_type = "hex";
    options.upper_bound = dochash;
    options.lower_bound = dochash;
    options.limit = 1;

    const result = await rpc.get_table_rows(options);
    if (result.rows.length > 0) {
        return result.rows[0];
    } 

    console.log ("There is no document with hash: ", dochash);
    return undefined;  
}
  
async function getDocuments (host, contract) {
    let rpc;
    let options = {};

    rpc = new JsonRpc(host, { fetch });
    options.code = contract;
    options.json = true;
    options.scope = contract;
    options.table = "documents";
    options.limit = 1000;

    const result = await rpc.get_table_rows(options);
    if (result.rows.length > 0) {
        return result.rows;
    } 

    console.log ("There are no documents");
    return undefined;  
}

async function sendtrx (prod, host, contract, action, authorizer, data) {
    const rpc = new JsonRpc(host, { fetch });
    var defaultPrivateKey;
    if (prod) { defaultPrivateKey = process.env.PRIVATE_KEY; }
    else defaultPrivateKey = "5KeSkosYfKj8yQRRvfWsTubTJrhBvEZixFYN8z7CfQmxf5nrjsY";
    const signatureProvider = new JsSignatureProvider([defaultPrivateKey]);
    const api = new Api({rpc, signatureProvider, textDecoder: new TextDecoder(), textEncoder: new TextEncoder()});
    const actions = [{account: contract,name: action,authorization: [{actor: authorizer, permission: "active"}],data: data}];
    const result = await api.transact({actions: actions}, {blocksBehind: 3, expireSeconds: 30});
    console.log("Transaction Successfull : ", result.transaction_id);
}

async function main () {
    const host = "http://localhost:8888";
    // const doc = JSON.parse(fs.readFileSync("../test/examples/each-type.json", 'utf8'));
    // await sendtrx(0, host, "docs", "create", "docs", doc);
    
    // const docs = await getDocuments (host, "docs");
    // console.log ("Documents table: ", JSON.stringify(docs, null, 2));

    const singleDoc = await getDocument (host, "docs", "d7366c3e6b63aa8f3ff664d7713d2db6c6e54662153d50e582f2e6de54f659ea");
    console.log ("Single document: ", JSON.stringify(singleDoc, null, 2));
}

main ();