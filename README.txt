
Private key: 5KeSkosYfKj8yQRRvfWsTubTJrhBvEZixFYN8z7CfQmxf5nrjsY
Public key: EOS696y3uuryxgRRCiajXHBtiX9umXKvhBRGMygPa82HtQDrcDnE6

cleos create account eosio docs EOS696y3uuryxgRRCiajXHBtiX9umXKvhBRGMygPa82HtQDrcDnE6 EOS696y3uuryxgRRCiajXHBtiX9umXKvhBRGMygPa82HtQDrcDnE6
cleos set contract docs document

eosc tx create docs edit '{
    "hash": "7b5755ce318c42fc750a754b4734282d1fad08e52c0de04762cb5f159a253c24",
    "owner": "docs",
    "values": [
        {
            "key": "salary_amount",
            "value": [
                "asset",
                "100.00 USD"
            ]
        }
    ]
}' -p docs