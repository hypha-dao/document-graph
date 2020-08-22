const dgraph = require("dgraph-js");
const grpc = require("grpc");
const { Api, JsonRpc } = require("eosjs");
const fetch = require("node-fetch");

async function getDocument(host, contract, dochash) {
    let rpc;
    let options = {};

    rpc = new JsonRpc(host, { fetch });
    options.code = contract;
    options.json = true;
    options.scope = contract;
    options.table = "documents";
    options.index_position = 2;
    options.key_type = "sha256";
    // options.encode_type = "hex";
    options.upper_bound = dochash;
    options.lower_bound = dochash;
    options.limit = 1;

    const result = await rpc.get_table_rows(options);
    if (result.rows.length > 0) {
        return result.rows[0];
    }

    console.log("There is no document with hash: ", dochash);
    return undefined;
}

// Create a client stub.
function newClientStub() {
    return new dgraph.DgraphClientStub("localhost:9080", grpc.credentials.createInsecure());
}

// Create a client.
function newClient(clientStub) {
    return new dgraph.DgraphClient(clientStub);
}

// Drop All - discard all data and start from a clean slate.
async function dropAll(dgraphClient) {
    const op = new dgraph.Operation();
    op.setDropAll(true);
    await dgraphClient.alter(op);
}

// Set schema.
async function setSchema(dgraphClient) {
    const schema = `
    id: int @index(int) .
    hash: string @index(exact) .
    creator: string .
    created_date: string .
    updated_date: string .
    certificates: string .
    content: [string] .
    `;

    
    const op = new dgraph.Operation();
    op.setSchema(schema);
    await dgraphClient.alter(op);
}

async function transformToDgraph (document) {

}

// Create data using JSON.
async function createData(dgraphClient) {
    // Create a new transaction.
    const txn = dgraphClient.newTxn();
    try {
        const document = await getDocument("https://test.telos.kitchen", "docs.hypha", "3beb0980ddbca90e7516fc0396f98211c4b134843c0589a1dfe720f9ef031d2f");

        document["uid"] = "_:3beb0980ddbca90e7516fc0396f98211c4b134843c0589a1dfe720f9ef031d2f";

        // Create data.
        const p = document;

        // Run mutation.
        const mu = new dgraph.Mutation();
        mu.setSetJson(p);
        const response = await txn.mutate(mu);

        // Commit transaction.
        await txn.commit();

        // Get uid of the outermost object (person named "Alice").
        // Response#getUidsMap() returns a map from blank node names to uids.
        // For a json mutation, blank node label is used for the name of the created nodes.
        console.log(`Created person named "Alice" with uid = ${response.getUidsMap().get("3beb0980ddbca90e7516fc0396f98211c4b134843c0589a1dfe720f9ef031d2f")}\n`);

        console.log("All created nodes (map from blank node names to uids):");
        response.getUidsMap().forEach((uid, key) => console.log(`${key} => ${uid}`));
        console.log();
    } finally {
        // Clean up. Calling this after txn.commit() is a no-op
        // and hence safe.
        await txn.discard();
    }
}

// Query for data.
async function queryData(dgraphClient) {
    // Run query.
    const query = `query all($a: string) {
        all(func: eq(hash, $a)) {
            uid
            creator
            content
        }
    }`;
    const vars = { $a: "3beb0980ddbca90e7516fc0396f98211c4b134843c0589a1dfe720f9ef031d2f" };
    const res = await dgraphClient.newTxn({ readOnly: true }).queryWithVars(query, vars);
    const ppl = res.getJson();

    // Print results.
    console.log(`Number of documents: ${ppl.all.length}`);
    ppl.all.forEach((person) => console.log(person));
}

async function main() {
    const dgraphClientStub = newClientStub();
    const dgraphClient = newClient(dgraphClientStub);
    await dropAll(dgraphClient);
    await setSchema(dgraphClient);
    await createData(dgraphClient);
    await queryData(dgraphClient);

    // Close the client stub.
    dgraphClientStub.close();
}

main().then(() => {
    console.log("\nDONE!");
}).catch((e) => {
    console.log("ERROR: ", e);
});