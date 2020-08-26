
class Util {
  static isString (value) {
    return typeof value === 'string' || value instanceof String
  }

  static toKeyValue (objs, key, value) {
    const keyValue = {}
    for (const obj of objs) {
      keyValue[obj[key]] = obj[value]
    }
    return keyValue
  }

  static removeDuplicates (values) {
    values = this.toArray(values)
    return [...new Set(values)]
  }

  static toArray (values) {
    return Array.isArray(values) ? values : [values]
  }

  static toUnderscoreCase (camelCase) {
    return camelCase.replace(/([A-Z])/g, '_$1').toLowerCase()
  }

  /**
   * a1 - a2 Ex. [1,2,3] - [2,3] = [1]
   * @param {*} a1
   * @param {*} a2
   */
  static arrayDiff (a1, a2) {
    return a1.filter(x => !a2.includes(x))
  }
}
module.exports = Util
