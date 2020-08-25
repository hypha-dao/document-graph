
const schema =
    `
      type Document {
          hash
          created_date
          creator
          content_groups
          certificates
      }
      
      type ContentGroup {
        content_group_sequence
        contents
      }
      
      type Content {
        label
        value
        type
        content_sequence
      }
      
      type Certificate {
        certifier
        notes
        certification_date
      }
      
      hash: string @index(exact) .
      created_date: datetime .
      creator: string @index(term) .
      content_groups: [uid] .
      certificates: [uid] .
      
      content_group_sequence: int .
      contents: [uid] .
      
      label: string @index(term) .
      value: string @index(term) .
      type: string @index(term) .
      content_sequence: int .
      
      certifier: string @index(term) .
      notes: string .
      certification_date: datetime .
    `

class Document {
  constructor (dgraph) {
    this.dgraph = dgraph
  }

  async setSchema () {
    return this.dgraph.updateSchema(schema)
  }

  async schemaExists () {
    return this.dgraph.typesExist(['Document', 'ContentGroup', 'Content', 'Certificate'])
  }

  async getByCreator (creator) {
    const { documents } = await this.dgraph.query(
      `query documents ($creator: string){
        documents(func: eq(creator, $creator)){
          expand(_all_){
            expand(_all_){
              expand(_all_)
            }
          }
        }
      }`,
      { $creator: creator }
    )
    return documents
  }

  async store (chainDoc) {
    const dgraphDoc = this._transform(chainDoc)
    return this.dgraph.updateData(dgraphDoc)
  }

  _transform (chainDoc) {
    const {
      hash,
      creator,
      created_date: createdDate,
      content_groups: contentGroups,
      certificates
    } = chainDoc

    return {
      hash,
      creator,
      created_date: createdDate,
      content_groups: this._transformContentGroups(contentGroups),
      certificates: this._transformCertificates(certificates),
      'dgraph.type': 'Document'

    }
  }

  _transformContentGroups (chainContentGroups) {
    const contentGroups = chainContentGroups.map((contentGroup, index) => {
      return {
        content_group_sequence: index,
        contents: this._transformContents(contentGroup),
        'dgraph.type': 'ContentGroup'
      }
    })
    return contentGroups
  }

  _transformContents (chainContents) {
    const contents = chainContents.map((content, index) => {
      const {
        label,
        value: [type, value]
      } = content
      return {
        label,
        value,
        type,
        content_sequence: index,
        'dgraph.type': 'Content'
      }
    })
    return contents
  }

  _transformCertificates (chainCertificates) {
    const certificates = chainCertificates.map((certificate) => ({
      ...certificate,
      'dgraph.type': 'Certificate'
    }))
    return certificates
  }
}

module.exports = Document
