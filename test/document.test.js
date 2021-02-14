/* eslint-disable prettier/prettier */
const assert = require('assert');
const eoslime = require('eoslime').init();
// eoslime.provider.defaultAccount => 

//     Account {
//         name: 'eosio',
//         executiveAuthority: {
//             actor: 'eosio', permission: 'active'
//         }
//         publicKey: 'EOS6MRyAjQq8ud7hVNYcfnVPJqcVpscN5So8BhtHuGYqET5GDW5CV',
//         privateKey: '5KQwrPbwdL6PhXujxW37FSSQZ1JiwsST4cqQzDeyXtP79zkvFD3',
//         provider: {
//              network: {
//                 "name": "local",
//                 "url": "http://127.0.0.1:8888",
//                 "chainId": "cf057bbfb72640471fd910bcb67639c22df9f92470936cddc1ade0e2f2e7dc4f"
//             },
//             ...
//         }
//     }

const fs = require('fs');

const DOCUMENT_WASM_PATH = 'build/docs/docs.wasm';
const DOCUMENT_ABI_PATH = 'build/docs/docs.abi';

 
describe('Document Testing', function () {

    // Increase mocha(testing framework) time, otherwise tests fails
    this.timeout(150000);

    let documentContract, documentAccount, user1; 
    let accounts;
    let config;
    
    before(async () => {

        console.log (JSON.stringify(eoslime, null, "  "))

        accounts = await eoslime.Account.createRandoms(3);
        documentAccount     = accounts[0];
        user1               = accounts[1];
      
        console.log (" Document Account     : ", documentAccount.name);
        console.log (" user1                : ", user1.name);

        documentContract = await eoslime.Contract.deploy (DOCUMENT_WASM_PATH, DOCUMENT_ABI_PATH, { inline: true });

        await documentAccount.addPermission('eosio.code');
        var documentOwner = eoslime.Account.load(documentAccount.name, documentAccount.privateKey, 'owner');
        await documentOwner.addPermission('eosio.code');

    });

    beforeEach(async () => {
        
    });

    it('Should create a document', async () => {

        
        
        const doc = JSON.parse(fs.readFileSync("test/examples/simplest.json", 'utf8'));
        console.log ("\nCreating document : ", doc);

        await documentContract.actions.create (user1.name, doc, { from: user1 });

        // const docs = await documentContract.provider.eos.getTableRows({
        //     code: documentAccount.name,
        //     scope: documentAccount.name,
        //     table: 'documents',
        //     json: true
        // });
        // console.log ("Documents table: ", JSON.stringify(docs, null, 2));
        // assert.equal (docs.rows[0].owner, user1.name);
    });
     
});