

export default interface initialGarageStateInterface {
  status: garageStatus,
  door_error: string,
  light_error: string,
  status_error: string,
  message: string
}

interface garageStatus {
  door: string,
  light: string,
  temperature: string,
}
