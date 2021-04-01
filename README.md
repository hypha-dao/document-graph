# Document Graph data structure

## Document 

Each document is comprised of:

- Header 
    - creator (account)
    - contract (where this is saved)
    - created date (timepoint)
    - hash of content (not including certificates or header)
- Content
    - FlexValue = ```std::variant <asset, string, time_point, name, int64> ```
    - Content = an optionally labeled FlexValue
    - ContentGroup = vector<Content>
    - ContentGroups = vector<ContentGroup>
    - there is a single instance of ContentGroups per document
    - this provides enough flexibility to support: 
        - data of all EOSIO types,
        - short clauses of annotated text,
        - longer form sequenced data, e.g. chapters 
- Certificates
    - each document has a list of certificates
    - Certificate 
        - certifier: the 'signer' 
        - notes: string data provided by signer
        - certification_date: time_point    

The simplest example:
```
{
   "id":4965,
   "hash":"50be6cf143050a11e9db3a52ef68e10e07b07cf6cc68007ad46a14baf307c5b9",
   "creator":"mem2.hypha",
   "content_groups":[
      [
         {
            "label":"simplest_label",
            "value":[
               "string",
               "Simplest"
            ]
         }
      ]
   ],
   "certificates":[],
   "created_date":"2021-01-12T18:21:10.000",
   "contract":"dao.hypha"
}
```

The "value" in each content item is a two element array, where the first item is the type and the second item is the data value.  The supported values are string, int64, asset, name, time_point, or checksum256.  

This contract uses [content addressing](https://flyingzumwalt.gitbooks.io/decentralized-web-primer/content/avenues-for-access/lessons/power-of-content-addressing.html), meaning the unique identifier of each document is a hash of its contents.  Each hash must be unique in the table and this is enforced by the actions.  

# Graph structure
Documents can be linked together with labeled, directional edges to create a graph.  For example, one document may be a "member" (vertex) that has an edge (link) to another document for a "role".  

![image](https://user-images.githubusercontent.com/32852271/90341301-73f54e00-dfcc-11ea-8022-587beaf8fedd.png)

Certificates are signed notes on documents by any account. Each certificate contains the account, timestamp, and an optional note. 

# Usage
This repo is meant to be used as a library in other smart contracts. It also includes a sample smart contract, a Go package/smart contract test package, and example cleos commands. It also has a nodejs script that does quite a bit but has not been well maintained. 

## Local Testing
- Install Go (https://golang.org/dl/)
- Install eosio & eosio.cdt

```
git clone https://github.com/hypha-dao/document-graph
cd document-graph
mkdir build
cd build
cmake ..
make -j8
cd ../docgraph
go test -v -timeout 0
```

## cleos Quickstart 
NOTE: Assumes you have relevant environmnent setup..
``` bash
# use your key
KEY=EOS696y3uuryxgRRCiajXHBtiX9umXKvhBRGMygPa82HtQDrcDnE6
cleos create account eosio documents $KEY $KEY
cleos create account eosio bob $KEY $KEY
cleos create account eosio alice $KEY $KEY
cleos set contract documents docs
```

You'll need to add the eosio.code permission (use your key)
``` bash
cleos push action eosio updateauth '{
    "account": "documents",
    "permission": "active",
    "parent": "owner",
    "auth": {
        "keys": [
            {
                "key": "EOS696y3uuryxgRRCiajXHBtiX9umXKvhBRGMygPa82HtQDrcDnE6",
                "weight": 1
            }
        ],
        "threshold": 1,
        "accounts": [
            {
                "permission": {
                    "actor": "documents",
                    "permission": "eosio.code"
                },
                "weight": 1
            }
        ],
        "waits": []
    }
}' -p documents@owner
```

``` bash
# this content just illustrates the various types supported
cleos push action documents create '{
    "creator": "bob",
    "content_groups": [
        [
            {
                "label": "content_group_name",
                "value": [
                    "string",
                    "My Content Group #1"
                ]
            },
            {
                "label": "salary_amount",
                "value": [
                    "asset",
                    "130.00 USD"
                ]
            },
            {
                "label": "referrer",
                "value": [
                    "name",
                    "friendacct"
                ]
            },
            {
                "label": "vote_count",
                "value": [
                    "int64",
                    67
                ]
            }
        ]
    ]
}' -p bob
```

Alice can fork the object. The content must be new or updated or else the action will fail and report back the hash. 
Only updated fields and the hash to the parent will be saved within a fork.
``` bash
cleos push action documents fork '{
    "hash": "",
    "creator": "alice",
    "content": [
        {
            "key": "salary_amount",
            "value": [[
                "asset",
                "150.00 USD"
            ]]
        }
    ]
}' -p alice
```


Any account can 'certify' a document, with notes.
``` bash
cleos push action documents certify '{
    "certifier": "documents",
    "hash": "b0477c431b96fa65273cb8a5f60ffb1fd11a42cb05d6e19cf2d66300ad52b8c9",
    "notes": "my certification notes"
}' -p documents
```


## Javascript Quickstart - DEPRECATED
Some of this will still work, but it's been replaced with the Go libraries and [daoctl](hypha-dao/daoctl).
``` bash
git clone git@github.com:hypha-dao/document.git
cd js && yarn install && node index.js
```

#### Create a document from a file
``` bash
$ node index.js --file "examples/each-type.json" --create --auth alice
Transaction Successfull :  7dc613a7c716897f498c95e5973333db5e6a9f5170f604cdcde1b4bb546bdef6
Documents table:  [
  {
    id: 0,
    hash: 'b0477c431b96fa65273cb8a5f60ffb1fd11a42cb05d6e19cf2d66300ad52b8c9',
    creator: 'alice',
    content: [ [Object], [Object], [Object], [Object], [Object], [Object] ],
    certificates: [],
    created_date: '2020-08-15T22:39:40.500',
    updated_date: '2020-08-15T22:39:40.500'
  }
]
```
NOTE: if you tried to recreate the same content a second time, it would fail to enforce in strict deduplication. This is similar to IPFS/IPLD specifications. There are more sample documents in the examples folder.

#### List documents
```
node index.js 
```
NOTE: use ```--json``` to show the entire document

#### Certify an existing document
```
node index.js --certify 526bbe0d21db98c692559db22a2a32fedbea378ca25a4822d52e1171941401b7 --auth bob
```
Certificates are stored in the same table as the content, but it is separate from the hashed content.

#### Add an edge
Creates a graph edge from a document to another document.
```
node js/index.js --link --from e91c036d9f90a9f2dc7ab9767ea4aa19c384431a24e45cf109b4fded0608ec99 --to c0b0e48a9cd1b73ac924cf58a430abd5d3091ca7cbcda6caf5b7e7cebb379327 --edge edger --contract documents --host http://localhost:8888 --auth alice 
```

#### Remove Edges
Edges can be removed using any of these options: 
1) one at a time (combination of from, to, and edge name), 
2) all edges for a specific from and to nodes, or
3) all edges for a specific from node and edge name.
 
### Document fingerprint
The document fingerprinting algorithm creates a data structure like this to hash.
```
[
    [children=[
        [checksum256,7b5755ce318c42fc750a754b4734282d1fad08e52c0de04762cb5f159a253c24],
        [checksum256,2f5f8a7c18567440b244bcc07ba7bb88cea80ddb3b4cbcb75afe6e15dd9ea33b]
    ],
    [description=[
        [string,loreum ipsum goes to the store, could also include markdown]
    ],
    [milestones=[
        [time_point,1597507314],
        [time_point,1597852914]
    ],
    [referrer=[
        [name,friendacct],
        [int64,67]
    ],
    [salary_amount=[
        [asset,130.00 USD]
    ],
    [vote_count=[
        [int64,69]
    ]
]
```
