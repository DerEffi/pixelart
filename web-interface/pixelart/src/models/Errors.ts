export enum APIErrorType {
    ConnectionError,
    UnauthorizedError,
    DeviceAddressError,
    AuthPending
}

export class APIError extends Error {

    constructor(type: APIErrorType = APIErrorType.ConnectionError, message: string = "Unexpected Error") {
        super();
        this.type = type;
        this.message = message;
    }

    public type: APIErrorType;
    public message: string;
}