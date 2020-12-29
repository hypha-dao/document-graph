
Setting up prometheus:

docker run -d -p 9090:9090 --name prometheus -v ~/monitoring/prometheus.yml:/etc/prometheus/prometheus.yml prom/prometheus
docker run -d -p 9100:9100 --name node-exporter prom/node-exporter:latest
docker run -d -p 8085:8085 --name daoctl_serve_prometheus dsmaxds/daoctl:latest

KEY=EOS696y3uuryxgRRCiajXHBtiX9umXKvhBRGMygPa82HtQDrcDnE6
cleos create account eosio documents $KEY $KEY
cleos create account eosio bob $KEY $KEY
cleos create account eosio alice $KEY $KEY
cleos set contract documents docs

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

node js/index.js --file "test/examples/each-type.json" --create --auth alice -h http://localhost:8888 --contract documents

node js/index.js --file "test/examples/simplest.json" --create --auth alice -h http://localhost:8888 --contract documents

node js/index.js --link --from ad10b49437f75ca3bbc3b762fa4e2c10286c3ece22b08f9d9313d5646ebd0e79 --to c0b0e48a9cd1b73ac924cf58a430abd5d3091ca7cbcda6caf5b7e7cebb379327 --edge edger --contract documents --host http://localhost:8888 --auth alice 


cleos push action documents removeedge '["ad10b49437f75ca3bbc3b762fa4e2c10286c3ece22b08f9d9313d5646ebd0e79", "c0b0e48a9cd1b73ac924cf58a430abd5d3091ca7cbcda6caf5b7e7cebb379327", "edger", 1]' -p alice

cleos push action documents removeedge '["ad10b49437f75ca3bbc3b762fa4e2c10286c3ece22b08f9d9313d5646ebd0e79", "c0b0e48a9cd1b73ac924cf58a430abd5d3091ca7cbcda6caf5b7e7cebb379327", "edger", 1]' -p alice

cleos push action documents removeedgest '["ad10b49437f75ca3bbc3b762fa4e2c10286c3ece22b08f9d9313d5646ebd0e79", "c0b0e48a9cd1b73ac924cf58a430abd5d3091ca7cbcda6caf5b7e7cebb379327", 1]' -p alice

cleos push action documents removeedgese '["ad10b49437f75ca3bbc3b762fa4e2c10286c3ece22b08f9d9313d5646ebd0e79", "edger", 1]' -p alice


node js/index.js --file "test/examples/simplest.json" --getorcreate --auth alice -h http://localhost:8888 --contract documents

node js/index.js --file "test/examples/simplest.json" --create --auth alice -h http://localhost:8888 --contract documents


// invalid link - should fail
node js/index.js --link --from ad10b49437f75ca3bbc3b762fa4e2c10286c3ece22b08f9d9313d5646ebd0e79 --to c0b0e48a9cd1b73ac924cf58a430abd5d3091ca7cbcda6caf5c7e7cebb379327 --edge edger --contract documents --host http://localhost:8888 --auth alice 


// testnet

node js/index.js --link --from ad10b49437f75ca3bbc3b762fa4e2c10286c3ece22b08f9d9313d5646ebd0e79 --to 120794413ba898001887db593afa7b4f556cbf2fb63325dd7700560936fe7fd0 --edge proposal --auth johnnyhypha1