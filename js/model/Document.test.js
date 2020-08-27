/* eslint-disable no-undef */
const DGraph = require('../service/DGraph')
const Document = require('./Document')

jest.setTimeout(20000)

let dgraph = null
let document = null

beforeAll(async () => {
  console.log('Before all')
  dgraph = new DGraph({
  })
  console.log('Dropping all')
  await dgraph.dropAll()
  console.log('Creating document')
  document = new Document(dgraph)
  console.log('Created document')
})

describe('Test store', () => {
  test('store', async () => {
    await document.setSchema()
    const missingTypes = await document.schemaExists()
    expect(missingTypes).toBeNull()
    await document.store(
      {
        id: 10,
        hash: 'c0b0e48a9cd1b73ac924cf58a430abd5d3091ca7cbcda6caf5b7e7cebb379327',
        creator: 'johnnyhypha1',
        content_groups: [
          [
            {
              label: 'content_group_name',
              value: [
                'string',
                'My Content Group #1'
              ]
            },
            {
              label: 'salary_amount',
              value: [
                'asset',
                '130.00 USD'
              ]
            },
            {
              label: 'referrer',
              value: [
                'name',
                'friendacct'
              ]
            },
            {
              label: 'vote_count',
              value: [
                'int64',
                67
              ]
            },
            {
              label: 'reference_link',
              value: [
                'checksum256',
                '7b5755ce318c42fc750a754b4734282d1fad08e52c0de04762cb5f159a253c24'
              ]
            }
          ],
          [
            {
              label: 'content_group_name',
              value: [
                'string',
                'My Content Group #2'
              ]
            },
            {
              label: 'salary_amount',
              value: [
                'asset',
                '130.00 USD'
              ]
            },
            {
              label: 'referrer',
              value: [
                'name',
                'friendacct'
              ]
            },
            {
              label: 'vote_count',
              value: [
                'int64',
                67
              ]
            },
            {
              label: 'reference_link',
              value: [
                'checksum256',
                '7b5755ce318c42fc750a754b4734282d1fad08e52c0de04762cb5f159a253c24'
              ]
            }
          ]
        ],
        certificates: [
          {
            certifier: 'alice',
            notes: "Alice's notes",
            certification_date: '2020-08-26T03:02:10.000'
          },
          {
            certifier: 'maria',
            notes: "Marias's notes",
            certification_date: '2020-08-26T03:02:15.000'
          }
        ],
        created_date: '2020-08-25T03:02:10.000'
      }
    )
    const documents = await document.getByCreator('johnnyhypha1')
    expect(documents).not.toBeNull()
    expect(documents).toBeInstanceOf(Array)
    expect(documents).toHaveLength(1)

    const uid = await document.getUID('c0b0e48a9cd1b73ac924cf58a430abd5d3091ca7cbcda6caf5b7e7cebb379327')
    expect(uid).not.toBeNull()
  })
})

afterAll(() => {
  dgraph.close()
})
