/* eslint-disable prettier/prettier */
const assert = require('assert');
const eoslime = require('eoslime').init('local');
const fs = require('fs');

const DOCUMENT_WASM_PATH = '../document/document.wasm';
const DOCUMENT_ABI_PATH = '../document/document.abi';

 
// async function getBalance (document, redeemer) {
//     const balances = await document.provider.eos.getTableRows({
//         code: document.name,
//         scope: redeemer.name,
//         table: 'balances',
//         json: true
//     });
//     if (balances.rows.length == 0) {
//         return "0.00 HUSD";
//     }
//     return balances.rows[0].funds;
// }

// async function getSupply (tokenContract) {
//     const supply = await tokenContract.provider.eos.getTableRows({
//         code: tokenContract.name,
//         scope: 'HUSD',
//         table: 'stat',
//         json: true
//     });
//     if (supply.rows.length == 0) {
//         return "0.00 HUSD";
//     }
//     return supply.rows[0].supply;
// }

// async function getPayments (document) {
//     const payments = await document.provider.eos.getTableRows({
//         code: document.name,
//         scope: document.name,
//         table: 'payments',
//         json: true
//     });
//     return payments.rows;
// }

// async function getRedemptions (document) {
//     const redemptions = await document.provider.eos.getTableRows({
//         code: document.name,
//         scope: document.name,
//         table: 'redemptions',
//         json: true
//     });
//     return redemptions.rows;
// }

// async function assertTreasuryBalance (document, redeemer, balance) {
//     const actualBalance = await getBalance (document, redeemer);
//     assert.equal (actualBalance, balance);
// }

describe('Document Testing', function () {

    // Increase mocha(testing framework) time, otherwise tests fails
    this.timeout(150000);

    let documentContract, documentAccount, user1; 
    let accounts;
    let config;
    
    before(async () => {

        accounts = await eoslime.Account.createRandoms(3);
        documentAccount     = accounts[0];
        user1               = accounts[1];
      
        console.log (" Document Account     : ", documentAccount.name);
        console.log (" user1                : ", user1.name);

        documentContract = await eoslime.Contract.deployOnAccount (DOCUMENT_WASM_PATH, DOCUMENT_ABI_PATH, documentAccount);

        await documentAccount.addPermission('eosio.code');
        var documentOwner = await eoslime.Account.load(documentAccount.name, documentAccount.privateKey, 'owner');
        await documentOwner.addPermission('eosio.code');

        // const authorityInfo = await documentAccount.getAuthorityInfo();
        // authorityInfo.required_auth.accounts.push({ permission: { actor: documentAccount, permission: "eosio.code" }, weight: 1 });
        // await documentAccount.updateAuthority(this, "owner", "", authorityInfo.required_auth);

        // await documentAccount.provider.eos.transaction(tr => {
        //     tr.updateauth({
        //         account: this.name,
        //         permission: "owner",
        //         parent: "",
        //         auth: { permission: { actor: documentAccount, permission: "eosio.code" }, weight: 1 }
        //     }, { authorization: [this.executiveAuthority] });
    
        // }, { broadcast: true, sign: true, keyProvider: this.privateKey });
            
        // console.log ("\nTreasury Configuration -----------------------")
        // let configTable = await documentContract.provider.eos.getTableRows({
        //     code: documentContract.name,
        //     scope: documentContract.name,
        //     table: 'config',
        //     json: true
        // });
        // config = configTable.rows[0];
        // console.log (config, "\n\n");
    });

    beforeEach(async () => {
        
    });

    it('Should create a document', async () => {
        
        const doc = JSON.parse(fs.readFileSync("examples/each-type.json", 'utf8'));
        console.log ("\nCreating document : ", doc);

        await documentContract.create (user1.name, doc, { from: user1 });

        const docs = await documentContract.provider.eos.getTableRows({
            code: documentAccount.name,
            scope: documentAccount.name,
            table: 'documents',
            json: true
        });
        console.log ("Documents table: ", JSON.stringify(docs, null, 2));
        assert.equal (docs.rows[0].owner, user1.name);
        // assert.equal (redemptions.rows[0].amount_requested, "10.00 HUSD");
    });

      
});