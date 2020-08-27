const {
  DgraphClient,
  DgraphClientStub,
  Operation,
  Mutation,
  Request
} = require('dgraph-js')

const { Util } = require('../util')

const grpc = require('grpc')

class DGraph {
  constructor ({
    addr = null,
    credentials = null
  }) {
    addr = addr || 'localhost:9080'
    credentials = credentials || grpc.credentials.createInsecure()
    this.clientStub = new DgraphClientStub(addr, credentials)
    this.client = new DgraphClient(this.clientStub)
    this.client.setDebugMode(false)
  }

  async updateSchema (schema, runInBackground = false) {
    const op = new Operation()
    op.setSchema(schema)
    op.setRunInBackground(runInBackground)
    return this.client.alter(op)
  }

  async getTypes (typeList) {
    return this.query(
      `schema(type:[${typeList.join(',')}]){}`
    )
  }

  async typesExist (typeList) {
    const { types } = await this.getTypes(typeList)
    if (!types) {
      return typeList
    }
    const existing = types.map(type => type.name)
    const missing = Util.arrayDiff(typeList, existing)
    return missing.length ? missing : null
  }

  async dropAll () {
    const op = new Operation()
    op.setDropAll(true)
    return this.client.alter(op)
  }

  async updateData (jsonData) {
    const txn = this.newTxn()
    try {
      const mutation = new Mutation()
      mutation.setSetJson(jsonData)
      const response = await txn.mutate(mutation)
      await txn.commit()
      return response
    } finally {
      await txn.discard()
    }
  }

  async upsert (query, update, condition = null) {
    const txn = this.newTxn()
    try {
      const mutation = new Mutation()
      mutation.setSetJson(update)
      if (condition) {
        mutation.setCond(condition)
      }
      const req = new Request()
      req.setQuery(query)
      req.addMutations(mutation)
      const response = await txn.doRequest(req)
      await txn.commit()
      return response
    } finally {
      await txn.discard()
    }
  }

  async query (queryStr, vars = null) {
    const txn = this.newTxn(true)
    const results = await (vars ? txn.queryWithVars(queryStr, vars) : txn.query(queryStr))
    return results.getJson()
  }

  newTxn (readOnly = false) {
    return this.client.newTxn({
      readOnly
    })
  }

  close () {
    this.clientStub.close()
  }
}

module.exports = DGraph
