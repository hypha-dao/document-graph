/* eslint-disable no-undef */
const DGraph = require('./DGraph')

jest.setTimeout(20000)

let dgraph = null

beforeAll(async () => {
  console.log('Before all')
  dgraph = new DGraph({
  })
  await dgraph.dropAll()
})

describe('Test updateSchema', () => {
  test('updateSchema', async () => {
    await dgraph.updateSchema(`

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
    `)
    const missingTypes = await dgraph.typesExist(['Document', 'ContentGroup', 'Content'])
    expect(missingTypes).toBeNull()
  })
})

describe('Test updateData', () => {
  test('updateData', async () => {
    await dgraph.updateData([
      {
        'dgraph.type': 'Document',
        certificates: [
          {
            'dgraph.type': 'Certificate',
            certifier: 'alice',
            notes: "Alice's notes",
            certification_date: '2020-08-25T16:59:58.500'
          }
        ],
        created_date: '2020-08-24T16:59:58.500',
        hash: '363bccee26dd6ffaa8f107d8ceb6a666a34f1de978c57dcad487475d107b79e5',
        creator: 'johnnyhypha1',
        content_groups: [
          {
            'dgraph.type': 'ContentGroup',
            content_group_sequence: 0,
            contents: [
              {
                'dgraph.type': 'Content',
                label: 'content_group_name',
                value: 'My content group #1',
                type: 'string',
                content_sequence: 0
              },
              {
                'dgraph.type': 'Content',
                label: 'important_value',
                value: 42,
                type: 'int64',
                content_sequence: 1
              }
            ]
          },
          {
            'dgraph.type': 'ContentGroup',
            content_group_sequence: 1,
            contents: [
              {
                label: '',
                value: 'My content group #2',
                type: 'string',
                content_sequence: 0
              }
            ]
          }
        ]
      }
    ])
    const { documents } = await dgraph.query(
      `query documents ($creator: string){
        documents(func: eq(creator, $creator)){
          expand(_all_){
            expand(_all_){
              expand(_all_)
            }
          }
        }
      }`,
      { $creator: 'johnnyhypha1' }
    )
    expect(documents).not.toBeNull()
    expect(documents).toBeInstanceOf(Array)
    expect(documents).toHaveLength(1)
  })
})

describe('Test upsert', () => {
  test('upsert', async () => {
    const query = `
        query {
          doc as var(func: eq(hash, "463bccee26dd6ffaa8f107d8ceb6a666a34f1de978c57dcad487475d107b79e5"))
        }
    `
    await dgraph.upsert(query,
      {
        'dgraph.type': 'Document',
        uid: 'uid(doc)',
        created_date: '2020-08-24T16:59:58.500',
        hash: '463bccee26dd6ffaa8f107d8ceb6a666a34f1de978c57dcad487475d107b79e5',
        creator: 'johnnyhypha1',
        content_groups: [
          {
            'dgraph.type': 'ContentGroup',
            content_group_sequence: 0,
            contents: [
              {
                'dgraph.type': 'Content',
                label: 'content_group_name',
                value: 'My content group #1',
                type: 'string',
                content_sequence: 0
              }
            ]
          }
        ]
      })

    let { documents } = await dgraph.query(
        `query documents ($hash: string){
          documents(func: eq(hash, $hash)){
            expand(_all_){
              expand(_all_){
                expand(_all_)
              }
            }
          }
        }`,
        { $hash: '463bccee26dd6ffaa8f107d8ceb6a666a34f1de978c57dcad487475d107b79e5' }
    )
    // console.log(JSON.stringify(documents, null, 4))
    expect(documents).not.toBeNull()
    expect(documents).toBeInstanceOf(Array)
    expect(documents).toHaveLength(1)
    expect(documents[0].certificates).toBe(undefined)

    await dgraph.upsert(query,
      {
        'dgraph.type': 'Document',
        uid: 'uid(doc)',
        certificates: [
          {
            'dgraph.type': 'Certificate',
            certifier: 'alice',
            notes: "Alice's notes",
            certification_date: '2020-08-25T16:59:58.500'
          }
        ]
      }
    );
    ({ documents } = await dgraph.query(
      `query documents ($hash: string){
        documents(func: eq(hash, $hash)){
          expand(_all_){
            expand(_all_){
              expand(_all_)
            }
          }
        }
      }`,
      { $hash: '463bccee26dd6ffaa8f107d8ceb6a666a34f1de978c57dcad487475d107b79e5' }
    ))
    // console.log(JSON.stringify(documents, null, 4))
    expect(documents).not.toBeNull()
    expect(documents).toBeInstanceOf(Array)
    expect(documents).toHaveLength(1)
    const certificates = documents[0]
    expect(certificates).not.toBeNull()
    expect(certificates).not.toBeInstanceOf(Array)
    expect(documents).toHaveLength(1)
  })
})

afterAll(() => {
  dgraph.close()
})
