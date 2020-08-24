var faker = require('faker');
const { Api, JsonRpc } = require("eosjs");
const { JsSignatureProvider } = require("eosjs/dist/eosjs-jssig");
const fetch = require("node-fetch");
const { TextEncoder, TextDecoder } = require("util");
var crypto = require('crypto');

// const commandLineArgs = require("command-line-args");
// const commandLineUsage = require('command-line-usage');
// const fs = require('fs');

var library = [];
var rootDocument = true;

async function getLastCreatedHash(host, contract) {
    let rpc;
    let options = {};

    rpc = new JsonRpc(host, { fetch });
    options.code = contract;
    options.scope = contract;

    options.json = true;
    options.scope = contract;
    options.table = "documents";
    options.index_position = 4; // index #2 is "bycreated"
    options.key_type = 'i64';
    options.reverse = true;
    options.limit = 1;

    const result = await rpc.get_table_rows(options);
    if (result.rows.length > 0) {
        return result.rows[0].hash;
    }

    console.log("There are no documents");
    return undefined;
}

async function sendtrx(prod, host, contract, action, authorizer, data) {
    const rpc = new JsonRpc(host, { fetch });
    var defaultPrivateKey;
    if (prod) { defaultPrivateKey = process.env.PRIVATE_KEY; }
    else defaultPrivateKey = "5HwnoWBuuRmNdcqwBzd1LABFRKnTk2RY2kUMYKkZfF8tKodubtK";
    const signatureProvider = new JsSignatureProvider([defaultPrivateKey]);
    const api = new Api({ rpc, signatureProvider, textDecoder: new TextDecoder(), textEncoder: new TextEncoder() });
    const actions = [{ account: contract, name: action, authorization: [{ actor: authorizer, permission: "active" }], data: data }];
    const result = await api.transact({ actions: actions }, { blocksBehind: 3, expireSeconds: 30 });
    console.log("Transaction Successfull : ", result.transaction_id);
}

async function fakeChecksum() {
    if (library.length == 0) {
        var sha256 = crypto.createHash('sha256').update(faker.lorem.paragraphs()).digest('hex');
        library.push(sha256);
        return ["checksum256", sha256];   // TODO: random checksum256
    } else {
        return ["checksum256", library[Math.floor(Math.random() * library.length)]];
    }
}

async function fakeString() {
    return ["string", faker.lorem.paragraphs()];
}

async function fakeAsset() {
    return ["asset", faker.finance.amount() + " USD"];
}

async function fakeInt64() {
    return ["int64", Math.floor(Math.random() * 18000)];
}

async function fakeTimePoint() {
    return ["time_point", faker.date.future().toISOString().replace(/Z/, '')];
}

async function fakeName() {
    var length = 12;
    var result = '';
    var characters = 'abcdefghijklmnopqrstuvwxyz12345';
    var charactersLength = characters.length;
    for (var i = 0; i < length; i++) {
        result += characters.charAt(Math.floor(Math.random() * charactersLength));
    }
    return ["name", result];
}


async function fakeValue(typeIndex) {
    switch (typeIndex) {
        case 0:
            return await fakeString();

        case 1:
            return await fakeAsset();

        case 2:
            return await fakeTimePoint();

        case 3:
            if (rootDocument) {
                return await fakeString();
            }
            return await fakeChecksum();

        case 4:
            return await fakeInt64();

        case 5:
            return await fakeName();

    }
}

async function fakeContent() {
    var content = {};

    if (Math.floor(Math.random() * 10) == 1) {
        content["label"] = "";  // use no label 10% of the time
    } else {
        content["label"] = faker.lorem.word();
    }

    const types = ["string", "asset", "time_point", "checksum256", "int64", "name"];
    var typeIndex = Math.floor(Math.random() * types.length);
    content["value"] = await fakeValue(typeIndex);
    return content
}

async function fakeContentGroup() {
    var contentGroup = [];
    const contentCount = Math.floor(Math.random() * 12) + 1;
    for (var i = 0; i < contentCount; i++) {
        var content = await fakeContent();
        contentGroup.push(content);
    }

    return contentGroup;
}

async function fakeContentGroups() {
    var contents = [];
    const contentGroupCount = Math.floor(Math.random() * 3) + 1;
    for (var i = 0; i < contentGroupCount; i++) {
        var contentGroup = await fakeContentGroup();
        contents.push(contentGroup);
    }

    return contents;
}

const main = async () => {

    const documentCount = 10;
    for (var i = 0; i < documentCount; i++) {

        var document = {};
        document["content_groups"] = await fakeContentGroups();
        document["creator"] = "johnnyhypha1";

        await sendtrx(0, "https://test.telos.kitchen", "docs.hypha", "create", "johnnyhypha1", document);

        library.push(await getLastCreatedHash("https://test.telos.kitchen", "docs.hypha"));
        rootDocument = false;
    }
}

main()